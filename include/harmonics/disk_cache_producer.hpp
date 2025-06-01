#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "harmonics/core.hpp"
#include "harmonics/serialization.hpp"

namespace harmonics {

class DiskCacheProducer : public Producer {
  public:
    using TokenFn = std::function<std::string()>;

    DiskCacheProducer(std::shared_ptr<Producer> base, const std::string& name, TokenFn token = {})
        : base_{std::move(base)}, token_{std::move(token)} {
        const char* env = std::getenv("HARMONICS_CACHE_DIR");
        std::filesystem::path dir = env ? env : ".harmonics/cache";
        std::filesystem::create_directories(dir);
        cache_path_ = (dir / name).string();
        token_path_ = cache_path_ + ".token";

        std::string cur = token_ ? token_() : std::string{};
        if (!cur.empty()) {
            std::ifstream in(token_path_);
            std::string prev;
            if (in)
                std::getline(in, prev);
            if (prev != cur) {
                std::filesystem::remove(cache_path_);
                std::filesystem::remove(token_path_);
            }
        }

        if (!load_cache()) {
            populate_cache(base_size());
        } else if (records_.size() < base_size()) {
            append_cache(records_.size());
        }

        if (!cur.empty()) {
            std::ofstream out(token_path_, std::ios::trunc);
            out << cur;
        }
    }

    HTensor next() override {
        if (records_.empty())
            return {};
        if (index_ >= records_.size())
            index_ = 0;
        return records_[index_++];
    }

    std::size_t size() const override { return records_.size(); }

  private:
    std::shared_ptr<Producer> base_{};
    TokenFn token_{};
    std::string cache_path_{};
    std::string token_path_{};
    std::vector<HTensor> records_{};
    std::size_t index_{0};

    std::size_t base_size() { return base_ ? base_->size() : 0; }

    bool load_cache() {
        std::ifstream in(cache_path_, std::ios::binary);
        if (!in)
            return false;
        records_.clear();
        while (in.peek() != EOF) {
            try {
                records_.push_back(read_tensor(in));
            } catch (...) {
                break;
            }
        }
        index_ = 0;
        return !records_.empty();
    }

    void populate_cache(std::size_t count) {
        records_.clear();
        std::ofstream out(cache_path_, std::ios::binary);
        if (!out)
            return;
        for (std::size_t i = 0; i < count; ++i) {
            HTensor t = base_->next();
            records_.push_back(t);
            write_tensor(out, t);
        }
    }

    void append_cache(std::size_t start) {
        std::ofstream out(cache_path_, std::ios::binary | std::ios::app);
        if (!out)
            return;
        for (std::size_t i = start; i < base_size(); ++i) {
            HTensor t = base_->next();
            records_.push_back(t);
            write_tensor(out, t);
        }
    }
};

} // namespace harmonics
