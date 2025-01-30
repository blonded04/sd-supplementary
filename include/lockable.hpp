#pragma once

#include <cstdint>
#include <cstdio>

namespace lcks {

const std::size_t kCacheLineAlignment = 128u;

struct alignas(kCacheLineAlignment) lockable_t {
protected:
    static const inline std::uint64_t kSpinsThresholdBeforeYield = 500;

public:
    virtual void exclusive_lock() = 0;
    virtual void exclusive_unlock() = 0;

    virtual ~lockable_t() = default;
};

}  // namespace lcks
