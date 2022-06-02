#ifndef WIND_CLEANNER_
#define WIND_CLEANNER_

#include <functional>
#include <memory>
#include <utility>

namespace wind {
    class cleanner {
    public:
        ~cleanner() noexcept(false) = default;

    private:
        struct garbage;

        class unique_garbage {
        public:
            unique_garbage() : p{nullptr} {}

            template<typename Callable>
            unique_garbage(Callable&& callable, unique_garbage&& next)
                : p{new garbage{::std::forward<Callable>(callable), ::std::move(next)}} { }

            unique_garbage(const unique_garbage&) = delete;

            unique_garbage(unique_garbage&& oth) : p{::std::exchange(oth.p, nullptr)} { }

            unique_garbage& operator=(const unique_garbage& rhs) = delete;

            unique_garbage& operator=(unique_garbage&& rhs) {
                if(this != &rhs) {
                    if(p) { delete p; p = nullptr; }
                    p = ::std::exchange(rhs.p, nullptr);
                }
                return *this;
            }

            ~unique_garbage() noexcept(false) {
                if(p) { delete p; p = nullptr; }
            }

        private:
            garbage* p;

        };
        
        struct garbage {
            ::std::function<void()> clean;
            unique_garbage next;
            ~garbage() noexcept(false) { clean(); }
        };

        unique_garbage garbages;

        template<typename Callable, typename = decltype(::std::declval<Callable>()())>
        friend cleanner& operator << (cleanner& c, Callable&& callable) {
            unique_garbage new_garbage{::std::forward<Callable>(callable), ::std::move(c.garbages)};
            c.garbages = ::std::move(new_garbage);
            return c;
        }
    };
}

#ifdef ALLOW_WIND_CLEANNER_MACRO

#define DISCARD(CLEANNER) CLEANNER << [&]()
#define DISCARD_COPY(CLEANNER) CLEANNER << [=]()

#endif // ALLOW_WIND_CLEANNER_MACRO

#endif