#pragma once

#include "read_lockable.hpp"
#include "ttas_lock.hpp"

#include <atomic>
#include <stdexcept>
#include <thread>

namespace lcks {

struct seq_lock_t final : public read_lockable_t {
private:
    ttas_lock_t m_lock;
    std::atomic<std::uint64_t> m_epoch;

public:
    seq_lock_t() : m_lock(), m_epoch(0) {}

    void exclusive_lock() override {
        m_lock.exclusive_lock();
        m_epoch.fetch_add(1, std::memory_order_release);
    }

    void exclusive_unlock() override {
        m_epoch.fetch_add(std::memory_order_acquire);
    }

    static thread_local std::uint64_t m_shared_lock_epoch;

    void shared_lock() override {
        m_shared_lock_epoch = m_epoch.load(std::memory_order_acquire);
    }

    bool shared_unlock() override {
        std::uint64_t epoch = m_epoch.load(std::memory_order_acquire);
        return epoch % 2 == 0 && epoch == m_shared_lock_epoch;
    }
};

}  // namespace lcks
