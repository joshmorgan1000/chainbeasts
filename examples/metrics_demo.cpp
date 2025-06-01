#include "neuropet/training.hpp"
#include <chrono>
#include <harmonics/dataset.hpp>
#include <thread>

int main() {
    neuropet::MetricsStreamer streamer{"127.0.0.1", 8765};
    auto loader = std::make_shared<harmonics::HttpProducer>("127.0.0.1", 8080, "/train");
    auto labels = std::make_shared<harmonics::HttpProducer>("127.0.0.1", 8080, "/label");
    neuropet::production_training_pipeline(loader, labels, "train.cache", "label.cache", 10,
                                           streamer);
    return 0;
}
