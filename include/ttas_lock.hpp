#pragma once

#include <atomic>
#include <thread>
#include "lockable.hpp"

namespace lcks {

struct ttas_lock_t final : public lockable_t {
private:
    std::atomic<int> m_readers;
    std::atomic<bool> m_writer;

public:
    ttas_lock_t() : m_readers(0), m_writer(false) {
    }

    virtual void exclusive_lock() {
        while (true) {
            while (m_writer.load(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
            if (!m_writer.exchange(true, std::memory_order_acquire)) {
                while (m_readers.load(std::memory_order_acquire) != 0) {
                    std::this_thread::yield();
                }
                return;
            }
        }
    }

    virtual void exclusive_unlock() {
        m_writer.store(false, std::memory_order_release);
    }
};

}  // namespace lcks
