#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <atomic>
#include <thread>
#include <vector>
#include "doctest.h"
#include "ttas_lock.hpp"

TEST_CASE("Single thread lock and unlock") {
    lcks::ttas_lock_t lock;
    REQUIRE_NOTHROW(lock.exclusive_lock());
    REQUIRE_NOTHROW(lock.exclusive_unlock());
}

TEST_CASE("Exclusive access with multiple threads") {
    lcks::ttas_lock_t lock;
    std::atomic<int> counter = 0;
    const int num_threads = 5;

    auto increment = [&lock, &counter]() {
        lock.exclusive_lock();
        ++counter;
        lock.exclusive_unlock();
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    CHECK(counter == num_threads);
}

TEST_CASE("Lock contention test") {
    lcks::ttas_lock_t lock;
    std::atomic<int> shared_counter = 0;
    const int num_threads = 10;
    const int iterations = 100;

    auto task = [&lock, &shared_counter]() {
        for (int i = 0; i < iterations; ++i) {
            lock.exclusive_lock();
            ++shared_counter;
            lock.exclusive_unlock();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(task);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    CHECK(shared_counter == num_threads * iterations);
}

TEST_CASE("Exception safety") {
    lcks::ttas_lock_t lock;
    try {
        lock.exclusive_lock();
        throw std::runtime_error("Simulated exception during lock");
    } catch (...) {
        lock.exclusive_unlock();
    }

    REQUIRE_NOTHROW(lock.exclusive_lock());
    lock.exclusive_unlock();
}
