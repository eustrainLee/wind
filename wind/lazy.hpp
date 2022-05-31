#ifndef ___WIND_LAZY___
#define ___WIND_LAZY___

#include <variant>
#include <functional>

namespace wind {

    template<typename>
    class lazy;

    template<typename T>
    class lazy {
    public:
        template<typename Callable, typename... Args, typename = decltype(::std::declval<Callable>()(::std::forward<Args>(::std::declval<Args>())...))/*, typename ::std::enable_if<(sizeof...(Args) > 0), int>::type = 0*/>
        explicit lazy(Callable&& callable, Args&&... args) : storge{
            ::std::in_place_type<::std::function<T()>>,
            ::std::bind(::std::forward<Callable>(callable), ::std::forward<Args>(args)...)
        } {}

        template<typename Callable, typename = typename ::std::enable_if<::std::is_same_v<decltype(::std::declval<Callable>()()), T>,int>::type>
        /*implicit*/ lazy(Callable&& callable) : storge{
            ::std::in_place_type<::std::function<T()>>,
            ::std::forward<Callable>(callable)
        } {}

        explicit lazy(const T& value) : storge {
            ::std::in_place_type<T>,
            value
        } {}

        explicit lazy(T&& value) : storge {
            ::std::in_place_type<T>,
            value
        } {}

        T& value() {
            if(!::std::holds_alternative<T>(this->storge)) {
                this->storge.template emplace<T>(::std::get<::std::function<T()>>(this->storge)());
            }
            return ::std::get<T>(this->storge);
        }

        /*implicit*/ operator T&() {
            return this->value();
        }

        lazy<T>& operator=(const T& value) {
            (T&)*this = value;
            return *this;
        }

        lazy<T>& operator=(T&& value) {
            (T&)*this = ::std::move(value);
            return *this;
        }

    private:
        ::std::variant<T, ::std::function<T()>> storge;

    };

    template<typename T>
    class lazy<T&> {
    public:
        template<typename Callable, typename... Args, typename = decltype(::std::declval<Callable>()(::std::forward<Args>(::std::declval<Args>())...))>
        explicit lazy(Callable&& callable, Args&&... args) : storge{
            ::std::in_place_type<::std::function<T&()>>,
            ::std::bind(::std::forward<Callable>(callable), ::std::forward<Args>(args)...)
        } {}

        template<typename Callable, typename = typename ::std::enable_if<::std::is_same_v<decltype(::std::declval<Callable>()()), T&>,int>::type>
        /*implicit*/ lazy(Callable&& callable) : storge{
            ::std::in_place_type<::std::function<T&()>>,
            ::std::forward<Callable>(callable)
        } {}

        explicit lazy(T& value) : storge {
            ::std::in_place_type<::std::reference_wrapper<T>>,
            value
        } {}

        T& value() {
            if(!::std::holds_alternative<::std::reference_wrapper<T>>(this->storge)) {
                this->storge.template emplace<::std::reference_wrapper<T>>(::std::get<::std::function<T&()>>(this->storge)());
            }
            return ::std::get<::std::reference_wrapper<T>>(this->storge);
        }

        /*implicit*/ operator T&() {
            return this->value();
        }

        lazy<T&>& operator=(const T& value) {
            (T&)*this = value;
            return *this;
        }

        lazy<T&>& operator=(T&& value) {
            (T&)*this = ::std::move(value);
            return *this;
        }

    private:
        ::std::variant<::std::reference_wrapper<T>, ::std::function<T&()>> storge;
    };

    template<typename Callable>
    lazy(Callable&&) -> lazy<decltype(::std::declval<Callable>()())>;

} // namespace wind


#endif