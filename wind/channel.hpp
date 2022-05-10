#ifndef ___WIND_CHANNEL___
#define ___WIND_CHANNEL___
#include <cstdint>
#include <utility>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>
#include <cstdlib>

#include <chrono>
#include <iostream>

// #include "spinlock.hpp"
// #include "uninitialized_variable.hpp"

namespace wind {
    template<typename T>
    class channel {
    public:
        explicit channel()
            : channel(0) {}
        explicit channel(const ::std::size_t N)
            : storge(N == 0 ? nullptr : static_cast<T*>(::std::malloc(sizeof(T), sizeof(T)*(N))))
            , buffer_size(N)
            , front(storge), rear(storge)
            , count(0) {}
        ~channel() { // don't destruct it when it is used.
            if(buffer_size != 0) {
                while(!this->empty()) {
                    front->~T();
                    this->front.store(this->next(front), ::std::memory_order_relaxed);
                    --this->count;
                }
                ::std::free(storge);
            }
        }
        template<typename... Args, typename ::std::enable_if<::std::is_constructible_v<T, Args&&...>, int>::type = 0>
        void push(Args&&... args) {
            if(no_buffer()) { // no buffer
                push_without_buffer(::std::forward<Args>(args)...);
            } else { // buffered
                push_with_buffer(::std::forward<Args>(args)...);
            }
        }

        T pop() {
            if(no_buffer()) { // no buffer
                return pop_without_buffer();
            } else { // buffered
                return pop_with_buffer();
            }
        }

        bool empty() const { return this->count == 0; }

        bool full() const { return this->count == this->buffer_size; }

        ::std::size_t size() const { return this->count; }

        bool no_buffer() const { return this->buffer_size == 0; }

        ::std::size_t capacity() const { return this->buffer_size; }

        friend channel& operator<<(channel& chan, const T& elem) {
            chan.push(elem);
            return chan;
        }

        friend channel& operator<<(channel& chan, T&& elem) {
            chan.push(::std::move(elem));
            return chan;
        }

        friend channel& operator>>(channel& chan, T& elem) {
            elem = chan.pop();
            return chan;
        }

    private:
        template<typename... Args, typename ::std::enable_if<::std::is_constructible_v<T, Args&&...>, int>::type = 0>
        void push_without_buffer(Args&&... args) {
            push_without_buffer(T(::std::forward<Args>(args)...)); // TODO: Deferred construction
        }

        void push_without_buffer(T&& t) {
            std::unique_lock<std::mutex> un_lk(mu);
            cond.wait(un_lk, [this]()-> bool {
                    return this->storge == nullptr
                        && this->front.load(::std::memory_order_acquire) != nullptr
                        && this->rear.load(::std::memory_order_acquire) == nullptr;
                });
            this->storge = &t;
            rear.store(const_cast<T*>(channel::global_tag), ::std::memory_order_release);
            un_lk.unlock();
            cond.notify_all(); // wait pop
            while(!(rear.load(::std::memory_order_acquire) == nullptr));
            cond.notify_all(); // notify other waiting threads
        }

        T pop_without_buffer() {
            ::std::unique_lock<std::mutex> un_lk(mu);
            cond.wait(un_lk, [this]() ->bool {
                    front.store(const_cast<T*>(channel::global_tag), ::std::memory_order_release);
                    cond.notify_all();
                    return this->storge != nullptr && rear.load(::std::memory_order_acquire) != nullptr;
                });
            T temp(::std::move(*this->storge)); // move, expect NRVO
            this->storge = nullptr;
            front.store(nullptr, ::std::memory_order_relaxed);
            rear.store(nullptr, ::std::memory_order_release);
            un_lk.unlock();
            return temp;
        }

        template<typename... Args, typename ::std::enable_if<::std::is_constructible_v<T, Args&&...>, int>::type = 0>
        void push_with_buffer(Args&&... args) {
            push_with_buffer(T(::std::forward<Args>(args)...));
        }

        void push_with_buffer(T&& elem) {
            ::std::unique_lock<::std::mutex> un_lk(this->mu);
            this->cond.wait(un_lk, [this]()->bool {
                    return !this->full();
                });
            T* rear = this->rear.load(::std::memory_order_relaxed);
            (void)new(rear)T(::std::move(elem));
            this->rear.store(this->next(rear), ::std::memory_order_relaxed);
            ++this->count;
            un_lk.unlock();
            cond.notify_all();
        }
        
        T pop_with_buffer() {
            ::std::unique_lock<::std::mutex> un_lk(this->mu);
            this->cond.wait(un_lk, [this]()->bool {
                    return !this->empty();
                });
            T* front = this->front.load(::std::memory_order_relaxed);
            T elem(::std::move(*front));
            front->~T();
            this->front.store(this->next(front), ::std::memory_order_relaxed);
            --this->count;
            un_lk.unlock();
            cond.notify_all();
            return elem;
        }

        T* next(T* p) const noexcept {
            if(p == this->storge + this->buffer_size-1) {
                return this->storge;
            }
            return p + 1;
        }

    private:
        T* storge;
        ::std::size_t const buffer_size;    // size of buffer, it is zero when the channel has no buffer
        ::std::size_t count;
        ::std::atomic<T*> front;  // pop here(buffered only) ready tag(no buffered only)
        ::std::atomic<T*> rear;   // push here(buffered only) over tag(no buffered only)
        ::std::mutex mu; // yield the CPU, when it needs a long time to wait resource.
        ::std::condition_variable cond; // aweak threads which waiting.
        static const T* global_tag; // TODO change to constexpr
    };

    template<typename T>
    inline const T* channel<T>::global_tag = const_cast<T* const>(reinterpret_cast<T*>(const_cast<T**>(&global_tag)));

    template<typename T>
    channel<T> make_chan(::std::size_t N) {
        return channel<T>(N);
    }

} // namespace wind

#endif
