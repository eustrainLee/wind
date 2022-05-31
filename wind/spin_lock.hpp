#ifndef WIND_SPIN_LOCK_
#define WIND_SPIN_LOCK_

#include <atomic>

namespace wind {
    class spin_lock {
    public:
        explicit spin_lock() : flag{ATOMIC_FLAG_INIT} {}
        bool try_lock() {
            return !flag.test_and_set(::std::memory_order_acquire);
        }
        void lock() {
            while(flag.test_and_set(::std::memory_order_acquire));
        }
        void unlock() {
            flag.clear(::std::memory_order_release);
        }
    private:
        ::std::atomic_flag flag;
    };
} // namespace wind

#endif
