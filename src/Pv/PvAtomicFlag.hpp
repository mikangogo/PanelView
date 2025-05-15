#pragma once

#include <atomic>

class PvAtomicFlag
{
public:
    void Set(const bool value)
    {
        _flag.store(value, std::memory_order_release);
    }

    bool Get() const
    {
        return _flag.load(std::memory_order_acquire);
    }

private:
    std::atomic<bool> _flag = {false};
};
