namespace wind {
    template<typename Ty>
    union UninitializedVariable {
        Ty value;
        UninitializedVariable() { /*do nothing*/ }
        ~UninitializedVariable() { /*do nothing*/ }
        UninitializedVariable(const UninitializedVariable&) = delete; // dangerous
        UninitializedVariable(UninitializedVariable&&) = delete; // dangerous
        UninitializedVariable& operator=(const UninitializedVariable&) = delete; // dangerous
        UninitializedVariable& operator=(UninitializedVariable&&) = delete; // dangerous
    };
} // namespace wind