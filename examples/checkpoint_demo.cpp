#include "neuropet/training.hpp"
#include <harmonics/dataset.hpp>

int main() {
    neuropet::MetricsStreamer streamer{"127.0.0.1", 8765};
    auto loader = std::make_shared<harmonics::HttpProducer>("127.0.0.1", 8080, "/train");
    neuropet::run_dataset_checkpoint_pipeline(loader, "train.cache", "trainer.ckpt", 5, streamer);
    return 0;
}
