#include "neuropet/training.hpp"
#include <gtest/gtest.h>
#include <harmonics/dataset.hpp>

#ifdef __unix__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <cstdio>
#include <thread>

static harmonics::HTensor make_tensor(float a, float b) {
    harmonics::HTensor t{harmonics::HTensor::DType::Float32, {2}};
    t.data().resize(sizeof(float) * 2);
    float vals[2] = {a, b};
    std::memcpy(t.data().data(), vals, sizeof(vals));
    return t;
}

struct CountingProducer : harmonics::Producer {
    explicit CountingProducer(int n) : limit{n} {}
    harmonics::HTensor next() override {
        if (index >= limit)
            return {};
        float v = static_cast<float>(index++);
        std::vector<std::byte> data(sizeof(float));
        std::memcpy(data.data(), &v, sizeof(float));
        return harmonics::HTensor{harmonics::HTensor::DType::Float32, {1}, std::move(data)};
    }
    std::size_t size() const override { return limit; }
    int limit;
    int index{0};
};

TEST(CacheTest, HttpProducerCachesToDisk) {
#ifdef __unix__
    const char* cache = "http_cache.bin";
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_EQ(server_fd >= 0, true);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    ASSERT_EQ(::bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), 0);
    ASSERT_EQ(::listen(server_fd, 1), 0);
    socklen_t len = sizeof(addr);
    ASSERT_EQ(::getsockname(server_fd, reinterpret_cast<sockaddr*>(&addr), &len), 0);
    unsigned short port = ntohs(addr.sin_port);

    std::thread server([&]() {
        int client = ::accept(server_fd, nullptr, nullptr);
        char c;
        std::string req;
        while (::recv(client, &c, 1, 0) > 0) {
            req.push_back(c);
            if (req.size() >= 4 && req.compare(req.size() - 4, 4, "\r\n\r\n") == 0)
                break;
        }
        std::ostringstream payload;
        harmonics::write_tensor(payload, make_tensor(1.f, 2.f));
        std::string body = payload.str();
        std::string resp =
            "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n";
        ::send(client, resp.c_str(), resp.size(), 0);
        ::send(client, body.data(), body.size(), 0);
        ::close(client);
        ::close(server_fd);
    });

    {
        harmonics::HttpProducer prod("127.0.0.1", port, "/", true, cache);
        EXPECT_EQ(prod.size(), 1u);
        auto t = prod.next();
        const float* d = reinterpret_cast<const float*>(t.data().data());
        EXPECT_EQ(d[1], 2.f);
    }

    server.join();

    harmonics::HttpProducer from_cache("127.0.0.1", port, "/", true, cache);
    EXPECT_EQ(from_cache.size(), 1u);
    auto r = from_cache.next();
    const float* dr = reinterpret_cast<const float*>(r.data().data());
    EXPECT_EQ(dr[0], 1.f);
    std::remove(cache);
#endif
}

TEST(CacheTest, DiskCacheProducerUpdatesIncrementally) {
    const char* cache = "disk_cache.bin";
    int version = 0;
    auto token = [&]() { return std::to_string(version); };

    {
        auto base = std::make_shared<CountingProducer>(2);
        harmonics::DiskCacheProducer prod(base, cache, token);
        EXPECT_EQ(prod.size(), 2u);
    }

    version = 1; // remote invalidation
    {
        auto base = std::make_shared<CountingProducer>(3);
        harmonics::DiskCacheProducer prod(base, cache, token);
        EXPECT_EQ(prod.size(), 3u);
    }

    {
        auto base = std::make_shared<CountingProducer>(4);
        harmonics::DiskCacheProducer prod(base, cache, token);
        EXPECT_EQ(prod.size(), 4u);
        auto t = prod.next();
        const float* d = reinterpret_cast<const float*>(t.data().data());
        EXPECT_EQ(d[0], 0.f);
    }

    std::remove(cache);
    std::remove("disk_cache.bin.token");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
