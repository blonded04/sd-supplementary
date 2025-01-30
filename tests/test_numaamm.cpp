#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include "doctest.h"
#include "numaamm_lock.hpp"

TEST_CASE("Single exclusive lock and unlock") {
    lcks::numaamm_lock_t lock;
    REQUIRE_NOTHROW(lock.exclusive_lock());
    REQUIRE_NOTHROW(lock.exclusive_unlock());
}

TEST_CASE("Exclusive locking: single thread reentrant") {
    lcks::numaamm_lock_t lock;
    lock.exclusive_lock();
    lock.exclusive_unlock();
    REQUIRE_NOTHROW(lock.exclusive_lock());
    lock.exclusive_unlock();
}

TEST_CASE("Multiple threads attempting exclusive lock") {
    lcks::numaamm_lock_t lock;
    std::atomic<int> counter = 0;

    const int num_threads = 5;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&lock, &counter]() {
            lock.exclusive_lock();
            int local = counter.load();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            counter.store(local + 1);
            lock.exclusive_unlock();
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    CHECK(counter == num_threads);
}

TEST_CASE("Concurrent writers should be sequential") {
    lcks::numaamm_lock_t lock;
    std::atomic<bool> flag = false;

    auto writer_task = [&lock, &flag]() {
        lock.exclusive_lock();
        CHECK(flag == false);
        flag = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        flag = false;  // Reset flag
        lock.exclusive_unlock();
    };

    std::thread writer1(writer_task);
    std::thread writer2(writer_task);

    writer1.join();
    writer2.join();

    CHECK(flag == false);
}

TEST_CASE("exception safety in exclusive locking") {
    lcks::numaamm_lock_t lock;
    try {
        lock.exclusive_lock();
        throw std::runtime_error("Simulated exception inside locked section");
    } catch (...) {
        lock.exclusive_unlock();
    }

    REQUIRE_NOTHROW(lock.exclusive_lock());
    lock.exclusive_unlock();
}
