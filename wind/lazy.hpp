#ifndef ___WIND_LAZY___
#define ___WIND_LAZY___

#include <optional>
#include <future>
#include <functional>
namespace wind {
    template<typename T>
    class lazy;

    template<typename T>
    class lazy {
    public:
        lazy(::std::function<T()> source) : source(::std::move(source)), opti_val{::std::nullopt} {}
        bool has_value() noexcept(noexcept(opti_val.has_value())) { return opti_val.has_value(); }
        const T& value() const {
            ::std::call_once(this->init_flag, &lazy<T>::init, this);
            return *this->opti_val;
        }
        T& value() {
            return const_cast<T&>(const_cast<const lazy*>(this)->value());
        }
        T& operator*() {
            return this->value();
        }
        const T& operator*() const {
            return this->value();
        }
        void init() const {
            this->opti_val.emplace(this->source());
        }

    private:
        mutable ::std::once_flag init_flag;
        const ::std::function<T()> source;
        mutable ::std::optional<T> opti_val;
    };

    template<typename T>
    class lazy<T&> {
    public:
        lazy(::std::function<T&()> source) : source(::std::move(source)), opti_val{::std::nullopt} {}
        bool has_value() noexcept(noexcept(opti_val.has_value())) { return opti_val.has_value(); }
        T& value() const {
            ::std::call_once(this->init_flag, &lazy<T&>::init, this);
            return *this->opti_val;
        }
        T& value() {
            return const_cast<T&>(const_cast<const lazy*>(this)->value());
        }
        T& operator*() {
            return this->value();
        }
        T& operator*() const {
            return this->value();
        }
        void init() const {
            this->opti_val.emplace(::std::ref(this->source()));
        }

    private:
        mutable ::std::once_flag init_flag;
        const ::std::function<T&()> source;
        mutable ::std::optional<::std::reference_wrapper<T>> opti_val;
    };

} // namespace wind


#endif