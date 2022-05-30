#ifndef ___WIND_LAZY___
#define ___WIND_LAZY___

#include <optional>
#include <functional>

namespace wind {

    template<typename>
    class lazy;

    template<typename T>
    class lazy {
    public:
        template<typename Callable, typename... Args, typename = decltype(::std::declval<Callable>()(::std::forward<Args>(::std::declval<Args>())...))>
        explicit lazy(Callable&& callable, Args&&... args) : source{
            [source = ::std::bind(::std::forward<Callable>(callable), ::std::forward<Args>(args)...), opti_value = ::std::optional<T>{::std::nullopt}] () mutable -> T& {
                if(!opti_value) {
                    opti_value.emplace(source());
                }
                return *opti_value;
            }
        } {}

        template<typename Callable, typename = typename ::std::enable_if<::std::is_same_v<decltype(::std::declval<Callable>()()), T>,int>::type>
        /*implicit*/ lazy(Callable&& callable) : source{
            [source = ::std::forward<Callable>(callable), opti_value = ::std::optional<T>{::std::nullopt}] () mutable -> T&{
                if(!opti_value) {
                    opti_value.emplace(source());
                }
                return *opti_value;
            }
        } {}

        explicit lazy(const T& value) : source{
            [value] () mutable -> T& { return value; }
        } {}

        explicit lazy(T&& value) : source{
            [value = ::std::move(value)] () mutable -> T& { return value; }
        } {}

        /*implicit*/ operator T&() {
            return source();
        }

        lazy<T>& operator=(const T& value) {
            (int&)*this = value;
            return *this;
        }

        lazy<T>& operator=(T&& value) {
            (int&)*this = ::std::move(value);
            return *this;
        }

    private:
        ::std::function<T&()> source;

    };

    template<typename T>
    class lazy<T&> {
    public:
        template<typename Callable, typename... Args, typename = decltype(::std::declval<Callable>()(::std::forward<Args>(::std::declval<Args>())...))>
        explicit lazy(Callable&& callable, Args&&... args) : source{
            [source = ::std::bind(::std::forward<Callable>(callable), ::std::forward<Args>(args)...), opti_value = ::std::optional<::std::reference_wrapper<T>>{::std::nullopt}] () mutable -> T& {
                if(!opti_value) {
                    opti_value.emplace(source());
                }
                return *opti_value;
            }
        } {}

        template<typename Callable, typename = typename ::std::enable_if<::std::is_same_v<decltype(::std::declval<Callable>()()), T&>,int>::type>
        /*implicit*/ lazy(Callable&& callable) : source{
            [source = ::std::forward<Callable>(callable), opti_value = ::std::optional<::std::reference_wrapper<T>>{::std::nullopt}] () mutable -> T&{
                if(!opti_value) {
                    opti_value.emplace(source());
                }
                return *opti_value;
            }
        } {}

        explicit lazy(T& value) : source{
            [value = ::std::reference_wrapper<T>{value}] () -> T& { return value; }
        } {}

        /*implicit*/ operator T&() { return source(); }

        lazy<T&>& operator=(const T& value) {
            (int&)*this = value;
            return *this;
        }

        lazy<T&>& operator=(T&& value) {
            (int&)*this = ::std::move(value);
            return *this;
        }

    private:
        ::std::function<T&()> source;

    };

    template<typename Callable>
    lazy(Callable&&) -> lazy<decltype(::std::declval<Callable>()())>;

} // namespace wind


#endif