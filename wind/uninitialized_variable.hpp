#ifndef WIND_UNINITIALIZED_VARIABLE_
#define WIND_UNINITIALIZED_VARIABLE_

namespace wind {
    template<typename T>
    union uninitialized_variable {
        T value;
        uninitialized_variable() { /*do nothing*/ }
        ~uninitialized_variable() { /*do nothing*/ }
        uninitialized_variable(const uninitialized_variable&) = delete; // dangerous
        uninitialized_variable(uninitialized_variable&&) = delete; // dangerous
        uninitialized_variable& operator=(const uninitialized_variable&) = delete; // dangerous
        uninitialized_variable& operator=(uninitialized_variable&&) = delete; // dangerous
    };
} // namespace wind

#endif