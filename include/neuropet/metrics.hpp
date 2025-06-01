#pragma once

#include <cstdint>
#include <string>

#include "harmonics/websocket_io.hpp"

namespace neuropet {

/** Simple training metrics structure. */
struct TrainingMetrics {
    std::uint32_t step{0};
    float loss{0.0f};
};

/**
 * @brief Utility for streaming metrics over a WebSocket connection.
 *
 * Each ``TrainingMetrics`` record is packed into a two element float tensor and
 * sent using the Harmonics ``WebSocketConsumer``. The class is intentionally
 * minimal since the unit tests only require a lightweight metrics channel.
 */
class MetricsStreamer {
  public:
    MetricsStreamer(const std::string& host, unsigned short port,
                    const std::string& path = "/metrics")
        : socket_{host, port, path} {}

    /** Send metrics to the configured WebSocket endpoint. */
    void push(const TrainingMetrics& m) {
        harmonics::HTensor t{harmonics::HTensor::DType::Float32, {2}};
        t.data().resize(sizeof(float) * 2);
        float* d = reinterpret_cast<float*>(t.data().data());
        d[0] = static_cast<float>(m.step);
        d[1] = m.loss;
        socket_.push(t);
    }

  private:
    harmonics::WebSocketConsumer socket_;
};

} // namespace neuropet
