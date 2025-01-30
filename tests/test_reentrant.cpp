#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <atomic>
#include <thread>
#include <vector>
#include "doctest.h"
#include "reentrant_lock.hpp"

TEST_CASE("Single lock and unlock") {
    lcks::reentrant_lock_t lock;
    REQUIRE_NOTHROW(lock.exclusive_lock());
    REQUIRE_NOTHROW(lock.exclusive_unlock());
}

TEST_CASE("Reentrant locking in single thread") {
    lcks::reentrant_lock_t lock;
    lock.exclusive_lock();
    REQUIRE_NOTHROW(lock.exclusive_lock());
    lock.exclusive_unlock();
    REQUIRE_NOTHROW(lock.exclusive_unlock());
}

TEST_CASE("Multiple threads locking") {
    lcks::reentrant_lock_t lock;
    std::atomic<int> counter = 0;
    const int num_threads = 5;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&lock, &counter]() {
            lock.exclusive_lock();
            ++counter;
            lock.exclusive_unlock();
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    CHECK(counter == num_threads);
}

TEST_CASE("Reentrant lock excludes other threads") {
    lcks::reentrant_lock_t lock;
    std::atomic<int> shared_data = 0;
    const int increment_value = 1;

    auto safe_increment = [&lock, &shared_data]() {
        lock.exclusive_lock();
        int local_copy = shared_data;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        shared_data = local_copy + increment_value;
        lock.exclusive_unlock();
    };

    std::thread t1(safe_increment);
    std::thread t2(safe_increment);

    t1.join();
    t2.join();

    CHECK(shared_data == 2 * increment_value);
}

TEST_CASE("Exception safety") {
    lcks::reentrant_lock_t lock;
    try {
        lock.exclusive_lock();
        throw std::runtime_error("Simulate exception during locked operation");
    } catch (...) {
        REQUIRE_NOTHROW(lock.exclusive_unlock());
    }

    REQUIRE_NOTHROW(lock.exclusive_lock());
    lock.exclusive_unlock();
}
