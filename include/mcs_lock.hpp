#pragma once

#include "lockable.hpp"

#include <atomic>
#include <thread>

namespace lcks {

struct mcs_lock_t final : public lockable_t {
private:
    struct mcs_node_t {
        std::atomic<mcs_node_t*> next;
        std::atomic<bool> locked;
    };

    std::atomic<mcs_node_t*> m_tail;
    static thread_local mcs_node_t m_qnode;

public:
    mcs_lock_t() : m_tail(nullptr) {}

    void exclusive_lock() override {
        m_qnode.next.store(nullptr, std::memory_order_release);
        mcs_node_t *pred = m_tail.exchange(m_qnode.next.load(std::memory_order_relaxed), std::memory_order_acq_rel);
        if (pred != nullptr) {
            m_qnode.locked.store(true, std::memory_order_relaxed);
            pred->next.store(&m_qnode, std::memory_order_release);
            std::uint64_t spins = 0;
            while (m_qnode.locked.load(std::memory_order_acq_rel)) {
                spins++;
                if (spins >= kSpinsThresholdBeforeYield) {
                    std::this_thread::yield();
                }
            }
        }
    }

    void exclusive_unlock() override {
        mcs_node_t *succ = m_qnode.next.load(std::memory_order_acquire);
        if (succ == nullptr) {
            mcs_node_t* expected = &m_qnode;
            if (m_tail.compare_exchange_strong(expected, nullptr, std::memory_order_acq_rel)) {
                return;
            }
            std::uint64_t spins = 0;
            while (succ == nullptr) {
                succ = m_qnode.next.load(std::memory_order_acquire);
                spins++;
                if (spins >= kSpinsThresholdBeforeYield) {
                    std::this_thread::yield();
                }
            }
        }
        succ->locked.store(false, std::memory_order_release);
    }
};

}  // namespace lcks
