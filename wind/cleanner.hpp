#ifndef WIND_CLEANNER_
#define WIND_CLEANNER_

#include <functional>
#include <memory>

namespace wind {
    class cleanner {
        struct task {
            ::std::function<void()> clean;
            ::std::unique_ptr<task> next;
            ~task() { clean(); }
        };
        ::std::unique_ptr<task> tasks;
        template<typename Callable, typename = decltype(::std::declval<Callable>()())>
        friend cleanner& operator << (cleanner& c, Callable&& callable) {
            ::std::unique_ptr<task> new_task = ::std::unique_ptr<task>{new task{::std::forward<Callable>(callable), ::std::move(c.tasks)}};
            c.tasks = ::std::move(new_task);
            return c;
        }
    };
}

#ifdef ALLOW_WIND_CLEANNER_MACRO_

#define DISCARD(CLEANNER) CLEANNER << [&]()

#endif // ALLOW_WIND_CLEANNER_MACRO_

#endif