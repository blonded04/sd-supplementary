#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <atomic>
#include <thread>
#include <vector>
#include "doctest.h"
#include "mcs_lock.hpp"

thread_local lcks::mcs_lock_t::mcs_node_t lcks::mcs_lock_t::m_qnode;

TEST_CASE("Single thread lock and unlock") {
    lcks::mcs_lock_t lock{};
    REQUIRE_NOTHROW(lock.exclusive_lock());
    REQUIRE_NOTHROW(lock.exclusive_unlock());
}

TEST_CASE("Multiple threads exclusive access") {
    lcks::mcs_lock_t lock;
    std::atomic<int> counter = 0;
    const int num_threads = 5;
    const int num_iterations = 1000;

    auto work = [&lock, &counter]() {
        for (int i = 0; i < num_iterations; ++i) {
            lock.exclusive_lock();
            ++counter;
            lock.exclusive_unlock();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(work);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    CHECK(counter == num_threads * num_iterations);
}

TEST_CASE("No deadlock with back-to-back locks") {
    lcks::mcs_lock_t lock;
    std::atomic<int> value = 0;

    auto lock_twice = [&lock, &value]() {
        for (int i = 0; i < 10; ++i) {
            lock.exclusive_lock();
            value += 1;
            lock.exclusive_unlock();
            lock.exclusive_lock();
            value -= 1;
            lock.exclusive_unlock();
        }
    };

    std::thread t1(lock_twice);
    std::thread t2(lock_twice);

    t1.join();
    t2.join();

    CHECK(value == 0);
}

TEST_CASE("Stress-test with multiple threads") {
    lcks::mcs_lock_t lock;
    std::atomic<int> counter = 0;
    const int num_threads = 10;
    const int num_iterations = 10000;

    auto increment = [&lock, &counter]() {
        for (int i = 0; i < num_iterations; ++i) {
            lock.exclusive_lock();
            ++counter;
            lock.exclusive_unlock();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    CHECK(counter == num_threads * num_iterations);
}

TEST_CASE("Handoff Between Multiple Threads") {
    lcks::mcs_lock_t lock;
    std::atomic<int> shared_variable = 0;

    auto modify_shared_variable = [&lock, &shared_variable]() {
        lock.exclusive_lock();
        ++shared_variable;
        lock.exclusive_unlock();
    };

    std::thread t1(modify_shared_variable);
    std::thread t2(modify_shared_variable);
    std::thread t3(modify_shared_variable);

    t1.join();
    t2.join();
    t3.join();

    CHECK(shared_variable == 3);
}
