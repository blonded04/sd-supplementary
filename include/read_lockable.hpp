#pragma once

#include "lockable.hpp"

namespace lcks {

struct read_lockable_t : public lockable_t {
    virtual void shared_lock() = 0;
    // returns false, whether retry is required because of dirty-read
    virtual bool shared_unlock() = 0;
};

}  // namespace lcks
