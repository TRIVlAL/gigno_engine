#ifndef OFFSET_OF
#define OFFSET_OF

namespace gigno {

    template<typename TClass, typename TMember>
    inline size_t OffsetOf(TMember TClass::*member) {
        TClass object{};
        return (size_t)(&(object.*member))-(size_t)&object;
    }

}

#endif