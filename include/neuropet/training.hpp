#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "neuropet/int8_kernel.hpp"
#include "neuropet/int8_spec.hpp"
#include "neuropet/metrics.hpp"
#include <harmonics/dataset.hpp>
#if __has_include(<harmonics/disk_cache_producer.hpp>)
#include <harmonics/disk_cache_producer.hpp>
#endif
#include <harmonics/function_registry.hpp>
#include <harmonics/graph.hpp>
#include <harmonics/parser.hpp>
#include <harmonics/runtime.hpp>
#include <harmonics/shaders.hpp>
#include <limits>

namespace neuropet {

struct CreatureModel {
    Int8Network sensor;
    Int8Network core;
    Int8Network appendage;
};

constexpr std::size_t DEFAULT_MAX_MODEL_BYTES = 64 * 1024;

inline std::size_t network_byte_size(const Int8Network& net) {
    std::size_t bytes = 0;
    for (const auto& l : net.layers)
        bytes += l.weights.size() + l.bias.size();
    return bytes;
}

inline std::size_t model_byte_size(const CreatureModel& model) {
    return network_byte_size(model.sensor) + network_byte_size(model.core) +
           network_byte_size(model.appendage);
}

inline void enforce_model_size(const CreatureModel& model,
                               std::size_t max_bytes = DEFAULT_MAX_MODEL_BYTES) {
    if (model_byte_size(model) > max_bytes)
        throw std::runtime_error("model exceeds size limit");
}

inline int8_t clamp_int8(int32_t v) {
    return static_cast<int8_t>(std::max(-128, std::min(127, v)));
}

inline std::vector<int8_t> eval_network(const Int8Network& net, const std::vector<int8_t>& input) {
    std::vector<int8_t> cur = input;
    std::vector<int8_t> next;
    for (const auto& l : net.layers) {
        if (l.op == Int8Op::Dense) {
            next.resize(l.output);
            int8_matmul_gpu(cur, l.weights, next, 1, l.output, l.input);
            for (std::size_t i = 0; i < l.output; ++i)
                next[i] =
                    clamp_int8(static_cast<int32_t>(next[i]) + static_cast<int32_t>(l.bias[i]));
        } else {
            next.resize(cur.size());
            for (std::size_t i = 0; i < cur.size(); ++i)
                next[i] = cur[i] < 0 ? 0 : cur[i];
        }
        cur = next;
    }
    return cur;
}

inline std::vector<int8_t> run_inference(const CreatureModel& model,
                                         const std::vector<int8_t>& sensor_in) {
    enforce_model_size(model);
    auto s = eval_network(model.sensor, sensor_in);
    auto c = eval_network(model.core, s);
    return eval_network(model.appendage, c);
}

/**
 * Simple training loop emitting step metrics via a MetricsStreamer.
 * Executes a tiny HarmonicGraph and reports the gradient norm after each step.
 */
template <class Streamer> inline void train_with_metrics(std::size_t steps, Streamer& streamer) {
    using namespace harmonics;
    register_builtin_shaders();

    const char* src = R"(
producer input {1};
producer target {1} 1/1 input;
layer dense;
cycle {
  input -(relu)-> dense;
  dense <-(mse)- target;
}
)";

    Parser parser{src};
    auto ast = parser.parse_declarations();
    auto g = build_graph(ast);

    struct ConstProducer : Producer {
        float value;
        explicit ConstProducer(float v) : value{v} {}
        HTensor next() override {
            HTensor t{HTensor::DType::Float32, {1}};
            t.data().resize(sizeof(float));
            *reinterpret_cast<float*>(t.data().data()) = value;
            return t;
        }
        std::size_t size() const override { return 1; }
    };

    auto inp = std::make_shared<ConstProducer>(1.0f);
    auto lbl = std::make_shared<ConstProducer>(0.0f);
    g.bindProducer("input", inp);
    g.bindProducer("target", lbl);

    FitOptions opt;
    opt.learning_rate = 0.1f;

    TrainingMetrics m{};
    std::size_t step = 0;
    auto stop = [&](const CycleState& state) {
        float norm = gradients_l2_norm(state.weights);
        ++step;
        m.step = static_cast<std::uint32_t>(step);
        m.loss = norm;
        streamer.push(m);
        return step >= steps;
    };

    g.fit_until(stop, make_auto_policy(), opt);
}

inline void train_with_metrics(const CreatureModel& model, std::size_t steps,
                               const std::vector<int8_t>& sensor_in, MetricsStreamer& streamer) {
    enforce_model_size(model);
    TrainingMetrics m{};
    for (std::size_t i = 0; i < steps; ++i) {
        auto out = run_inference(model, sensor_in);
        m.step = static_cast<std::uint32_t>(i + 1);
        m.loss = static_cast<float>(out.empty() ? 0 : out[0]);
        streamer.push(m);
    }
}

/**
 * @brief Run a dataset through a DiskCacheProducer and stream metrics.
 *
 * Each sample is cached on disk under `.harmonics/cache` (or the directory
 * specified by `HARMONICS_CACHE_DIR`). The first float value of each tensor is
 * used as the loss metric for demonstration purposes.
 */
inline void run_dataset_pipeline(std::shared_ptr<harmonics::Producer> dataset,
                                 const std::string& cache_name, std::size_t steps,
                                 MetricsStreamer& streamer) {
    using namespace harmonics;
    register_builtin_shaders();

#if __has_include(<harmonics/disk_cache_producer.hpp>)
    auto cached = std::make_shared<harmonics::DiskCacheProducer>(dataset, cache_name);
    std::shared_ptr<harmonics::Producer> src = cached;
#else
    (void)cache_name;
    std::shared_ptr<harmonics::Producer> src = dataset;
#endif

    const char* src_graph = R"(
producer input {1};
producer target {1} 1/1 input;
layer dense;
cycle {
  input -(relu)-> dense;
  dense <-(mse)- target;
}
)";

    Parser parser{src_graph};
    auto ast = parser.parse_declarations();
    auto g = build_graph(ast);

    g.bindProducer("input", src);

    struct ZeroProducer : Producer {
        explicit ZeroProducer(std::size_t n) : n_{n} {}
        HTensor next() override {
            HTensor t{HTensor::DType::Float32, {1}};
            t.data().resize(sizeof(float));
            *reinterpret_cast<float*>(t.data().data()) = 0.0f;
            return t;
        }
        std::size_t size() const override { return n_; }
        std::size_t n_{};
    };

    auto lbl = std::make_shared<ZeroProducer>(src->size());
    g.bindProducer("target", lbl);

    FitOptions opt;
    opt.learning_rate = 0.1f;

    TrainingMetrics m{};
    std::size_t step = 0;
    auto stop = [&](const CycleState& state) {
        float norm = gradients_l2_norm(state.weights);
        ++step;
        m.step = static_cast<std::uint32_t>(step);
        m.loss = norm;
        streamer.push(m);
        return step >= steps;
    };

    g.fit_until(stop, make_auto_policy(), opt);
}

inline void production_training_pipeline(std::shared_ptr<harmonics::Producer> input,
                                         std::shared_ptr<harmonics::Producer> target,
                                         const std::string& input_cache,
                                         const std::string& target_cache, std::size_t steps,
                                         MetricsStreamer& streamer) {
    using namespace harmonics;
    register_builtin_shaders();
#if __has_include(<harmonics/disk_cache_producer.hpp>)
    auto cached_in = std::make_shared<harmonics::DiskCacheProducer>(input, input_cache);
    auto cached_target = std::make_shared<harmonics::DiskCacheProducer>(target, target_cache);
    std::shared_ptr<harmonics::Producer> src = cached_in;
    std::shared_ptr<harmonics::Producer> lbl = cached_target;
#else
    (void)input_cache;
    (void)target_cache;
    std::shared_ptr<harmonics::Producer> src = input;
    std::shared_ptr<harmonics::Producer> lbl = target;
#endif
    const char* src_graph = R"(
producer input {1};
producer target {1} 1/1 input;
layer dense;
cycle {
  input -(relu)-> dense;
  dense <-(mse)- target;
}
)";
    Parser parser{src_graph};
    auto ast = parser.parse_declarations();
    auto g = build_graph(ast);
    g.bindProducer("input", src);
    g.bindProducer("target", lbl);
    FitOptions opt;
    opt.learning_rate = 0.1f;
    TrainingMetrics m{};
    std::size_t step = 0;
    auto stop = [&](const CycleState& state) {
        float norm = gradients_l2_norm(state.weights);
        ++step;
        m.step = static_cast<std::uint32_t>(step);
        m.loss = norm;
        streamer.push(m);
        return step >= steps;
    };
    g.fit_until(stop, make_auto_policy(), opt);
}

} // namespace neuropet
