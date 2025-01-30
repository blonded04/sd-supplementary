#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <atomic>
#include <thread>
#include <vector>
#include "doctest.h"
#include "seq_lock.hpp"

namespace lcks {

thread_local std::uint64_t seq_lock_t::m_shared_lock_epoch = 0;

}  // namespace lcks

TEST_CASE("Single lock and unlock") {
    lcks::seq_lock_t lock{};
    REQUIRE_NOTHROW(lock.exclusive_lock());
    REQUIRE_NOTHROW(lock.exclusive_unlock());
}

TEST_CASE("Shared lock consistency") {
    lcks::seq_lock_t lock{};
    lock.shared_lock();
    CHECK(lock.shared_unlock() == true);
}

TEST_CASE("Concurrency test: writer then reader") {
    lcks::seq_lock_t lock{};
    std::atomic<int> share_var = 0;

    std::thread writer([&lock, &share_var]() {
        lock.exclusive_lock();
        share_var = 42;
        lock.exclusive_unlock();
    });

    std::thread reader([&lock, &share_var]() {
        int local_copy;
        lock.shared_lock();
        local_copy = share_var;
        if (!lock.shared_unlock()) {
            lock.shared_lock();
            local_copy = share_var;
            lock.shared_unlock();
        }
        CHECK(local_copy == 42);
    });

    writer.join();
    reader.join();
}

TEST_CASE("Multiple readers consistency") {
    lcks::seq_lock_t lock{};
    std::atomic<int> read_count = 0;
    const int num_readers = 10;

    std::vector<std::thread> readers;
    for (int i = 0; i < num_readers; ++i) {
        readers.emplace_back([&lock, &read_count]() {
            lock.shared_lock();
            ++read_count;
            if (!lock.shared_unlock()) {
                std::runtime_error("Read inconsistency detected");
            }
        });
    }

    for (auto &reader : readers) {
        reader.join();
    }

    CHECK(read_count == num_readers);
}

TEST_CASE("Writer excludes readers") {
    lcks::seq_lock_t lock{};
    std::atomic<bool> writer_active = false;
    std::atomic<bool> is_reader_saw_active_writer = false;

    const int num_readers = 5;

    std::thread writer([&lock, &writer_active]() {
        lock.exclusive_lock();
        writer_active = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        writer_active = false;
        lock.exclusive_unlock();
    });

    std::vector<std::thread> readers;
    for (int i = 0; i < num_readers; ++i) {
        readers.emplace_back([&lock, &writer_active,
                              &is_reader_saw_active_writer]() {
            lock.shared_lock();
            if (writer_active) {
                is_reader_saw_active_writer = true;
            }
            lock.shared_unlock();
        });
    }

    writer.join();
    for (auto &reader : readers) {
        reader.join();
    }

    CHECK(is_reader_saw_active_writer == false);
}
