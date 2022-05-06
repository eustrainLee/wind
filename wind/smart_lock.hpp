#ifndef ___WIND_SMART_LOCK___
#define ___WIND_SMART_LOCK___
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

namespace wind {
    class smart_lock {
    public:
        explicit smart_lock()
        : un_chair(chair, ::std::defer_lock), mu_count(0), waiting(false), use_mu(false), spin(false) {}
        ~smart_lock() {
            // if(spin.load() || mu_count.load(::std::memory_order_acquire) > 0) { /* do nothing */ } // undefined behavior
        }
        smart_lock(const smart_lock&) = delete;
        smart_lock(smart_lock&&) = delete;
        smart_lock& operator=(const smart_lock&) = delete;
        smart_lock& operator=(smart_lock&&) = delete;

    public:
        void lock() {
            if(!lock_spin_without_mu()) {
                mu_count.fetch_add(1, ::std::memory_order_acq_rel);
                mu.lock(); // 抢夺互斥锁
                lock_spin_with_mu();
                use_mu = true;
            }
        }

        bool try_lock() { // try_lock可用的前提是一定没有阻塞，此时一定使用spin_lock
            return try_lock_spin_without_mu();
        }

        void unlock() {
            bool need_unlock_mu = ::std::exchange(use_mu, false);
            unlock_spin(); // 先把自旋锁解开
            if(need_unlock_mu) { // 如果我有锁，不可能有人在等
                un_chair.unlock(); // 等待的那边可能虚假唤醒，但是无所谓，反正如果可能会导致虚假唤醒，那么这里接下来一定要notify_one，所以是有益的
            } // else: 没锁，可能有人在等 啥也不干，直接让渡自旋锁就得了，这件事已经完成了
            if(waiting.load(::std::memory_order_acquire)) { // 如果有线程正在等待，那么waiting一定为true
                cond.notify_one(); // 等待的线程至多一个，所以无需notify_all
            }
            if(need_unlock_mu) {
                mu.unlock();
                mu_count.fetch_sub(1, ::std::memory_order_acq_rel);
            }
        }

    private:
        bool lock_spin_without_mu() {
            int spin_attempts = 0;
            do {
                if(mu_count.load(::std::memory_order_acquire) > 0) {
                    return false;
                }
                for(int attempts = 0; attempts != max_attempts / 2; ++attempts) {
                    if(!spin.exchange(true)) { return true; }
                }
                if(mu_count.load(::std::memory_order_acquire) > 0) {
                    return false;
                }
                for(int attempts = 0; attempts != max_attempts / 2 + (max_attempts & 1); ++attempts) {
                    if(!spin.exchange(true)) { return true; }
                }
                ::std::this_thread::yield();
            } while(++spin_attempts < max_spin_attempts);
            return false;
        }

        bool try_lock_spin_without_mu() {
            if(mu_count.load(::std::memory_order_acquire) > 0) { return false; }
            return !spin.exchange(true);
        }

        void lock_spin_with_mu() {
            un_chair.lock();
            while(spin.exchange(true, ::std::memory_order_acq_rel)) { // 试图获得自旋锁，因为没有竞争所以没必要一直自旋
                waiting.store(true, ::std::memory_order_release);
                cond.wait(un_chair, [this]{ return !spin.load(::std::memory_order_acquire); }); // 失败了就等着
                waiting.store(false, ::std::memory_order_release);
            };
        }

        void unlock_spin() {
            spin.store(false);
        }

    private:
        inline static constexpr int max_attempts = 210;
        inline static constexpr int max_spin_attempts = 8;

    private:
        ::std::mutex mu;
        ::std::mutex chair;
        ::std::unique_lock<::std::mutex> un_chair;
        ::std::condition_variable cond;
        ::std::atomic_int mu_count;
        ::std::atomic_bool waiting;
        bool use_mu;
        ::std::atomic_bool spin;

    };
} // namespace wind
#endif