#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>
#include "lockable.hpp"

namespace lcks {

class reentrant_lock_t : public lockable_t {
private:
    std::mutex m_mutex;
    std::condition_variable m_read_cv;
    std::condition_variable m_write_cv;

    int m_read_count;

    std::thread::id m_writer_id;
    int m_write_count;

    std::unordered_map<std::thread::id, int> m_readers;

public:
    reentrant_lock_t() : m_read_count(0), m_writer_id(), m_write_count(0) {
    }

    void exclusive_lock() {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::thread::id this_id = std::this_thread::get_id();

        if (m_writer_id == this_id) {
            // If the current thread is the writer, just increase the counter
            ++m_write_count;
            return;
        }

        // Wait until there are no readers or writers
        m_write_cv.wait(lock, [this]() {
            return m_read_count == 0 && m_write_count == 0;
        });

        // Become the writer
        m_writer_id = this_id;
        m_write_count = 1;
    }

    void exclusive_unlock() {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::thread::id this_id = std::this_thread::get_id();

        if (m_writer_id == this_id) {
            --m_write_count;

            if (m_write_count == 0) {
                m_writer_id = std::thread::id();
                m_write_cv.notify_one();
                m_read_cv.notify_all();
            }
        }
    }
};
}  // namespace lcks
