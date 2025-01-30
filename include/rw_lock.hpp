#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "read_lockable.hpp"

namespace lcks {

class rw_lock_t : public read_lockable_t {
private:
    std::mutex m_mutex;
    std::condition_variable m_read_cv;
    std::condition_variable m_write_cv;

    int read_count;
    int write_count;

public:
    rw_lock_t() : read_count(0), write_count(0) {
    }

    void shared_lock() override {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_read_cv.wait(lock, [this]() { return write_count == 0; });

        ++read_count;
    }

    bool shared_unlock() override {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (read_count > 0) {
            --read_count;
            if (read_count == 0) {
                m_write_cv.notify_one();
            }
            return true;
        }
        return false;
    }

    void exclusive_lock() override {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_write_cv.wait(lock, [this]() {
            return read_count == 0 && write_count == 0;
        });

        ++write_count;
    }

    void exclusive_unlock() override {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (write_count > 0) {
            --write_count;
            m_read_cv.notify_all();
            m_write_cv.notify_one();
        }
    }
};
}  // namespace lcks
