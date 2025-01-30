#pragma once

#include <atomic>
#include <stdexcept>
#include <thread>
#include "lockable.hpp"
#include "ttas_lock.hpp"

namespace lcks {

struct numaamm_lock_t final : public lockable_t {
private:
    std::atomic<int> m_reader_count;
    std::atomic<bool> m_writer_flag;

public:
    numaamm_lock_t() : m_reader_count(0), m_writer_flag(false) {
    }

    void exclusive_lock() override {
        while (true) {
            while (m_writer_flag.load(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
            if (!m_writer_flag.exchange(true, std::memory_order_acquire)) {
                while (m_reader_count.load(std::memory_order_acquire) != 0) {
                    std::this_thread::yield();
                }
                return;
            }
        }
    }

    void exclusive_unlock() override {
        m_writer_flag.store(false, std::memory_order_release);
    }
};

}  // namespace lcks
