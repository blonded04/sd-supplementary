#pragma once

#include "lockable.hpp"

namespace lcks {

struct ttas_lock_t final : public lockable_t {
// TODO(kristina)
    ttas_lock_t() {}
};

}  // namespace lcks
