#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <atomic>
#include <thread>
#include <vector>
#include "doctest.h"
#include "rw_lock.hpp"

TEST_CASE("Single reader") {
    lcks::rw_lock_t lock;
    lock.shared_lock();
    REQUIRE(lock.shared_unlock() == true);
}

TEST_CASE("Single writer") {
    lcks::rw_lock_t lock;
    lock.exclusive_lock();
    lock.exclusive_unlock();
    REQUIRE(true);
}

TEST_CASE("Multiple readers") {
    lcks::rw_lock_t lock;
    std::atomic<int> counter = 0;
    const int num_threads = 10;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread([&lock, &counter]() {
            lock.shared_lock();
            ++counter;
            lock.shared_unlock();
        }));
    }

    for (auto &thread : threads) {
        thread.join();
    }

    REQUIRE(counter == num_threads);
}

TEST_CASE("Reader-writer interaction") {
    lcks::rw_lock_t lock;
    std::atomic<int> value = 0;

    std::thread writer([&lock, &value]() {
        lock.exclusive_lock();
        value = 42;
        lock.exclusive_unlock();
    });

    std::thread reader([&lock, &value]() {
        lock.shared_lock();
        while (value != 42) {
            lock.shared_unlock();
            std::this_thread::yield();
            lock.shared_lock();
        }
        lock.shared_unlock();
    });

    writer.join();
    reader.join();

    REQUIRE(value == 42);
}

TEST_CASE("Sequential writers") {
    lcks::rw_lock_t lock;
    std::atomic<int> sharedResource = 0;
    const int incrementValue = 5;

    std::vector<std::thread> writers;
    for (int i = 0; i < 3; ++i) {
        writers.push_back(std::thread([&lock, &sharedResource]() {
            lock.exclusive_lock();
            sharedResource += incrementValue;
            lock.exclusive_unlock();
        }));
    }

    for (auto &writer : writers) {
        writer.join();
    }

    REQUIRE(sharedResource == 3 * incrementValue);
}