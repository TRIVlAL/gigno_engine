#ifndef KEYVALUES_H
#define KEYVALUES_H

#include <cstddef>
#include "../algorithm/cstr_map.h"
#include <any>
#include <cstring>
#include "../../debug/console/console.h"
#include "../../stringify.h"
#include "glm/glm.hpp"
#include "../../algorithm/offset_of.h"
#include "../physics/collider_type.h"

namespace gigno {
    typedef glm::vec3 vec3;
    struct Transform_t;
    typedef const char * cstr;

    /*
    List here every types avaliable for a keyvalue
    Before you add types here are the requirements:
        - Type must specify ToString() and FromString() (see file stringify.h)
        - The type must be a single word, not separated by '::'. If not possible, simply typedef it.
    */
    #define TYPE_LIST\
    Type(int)\
    Type(float)\
    Type(bool)\
    Type(vec3)\
    Type(cstr)\
    Type(ColliderType_t)


    #define Type(t) type_##t,
    enum ValueType_t : int{
        TYPE_LIST
    };
    #undef Type


    // VALUE_TYPE correspond to the ValueType_t enum value of T.
    // Example : ValueTypeDeducer<int>::VALUE_TYPE == ValueType_t::type_int
    template<typename T> 
    class ValueTypeDeducer {
        enum { VALUE_TYPE };
    };

    #define VALUE_TYPE_DEDUCER(type) template<> class ValueTypeDeducer<type> { public: enum { VALUE_TYPE = ValueType_t::type_##type};}

    #define Type(t) VALUE_TYPE_DEDUCER(t);
    TYPE_LIST
    #undef Type


    struct OwnedValue_t {
        ValueType_t Type;

        //In bytes, the difference between the owners mem adress and the value.
        size_t Offset;

        size_t Size;
    };


    typedef CstrMap_t<OwnedValue_t> KeyValueMap_t;
    

    // Allows to access the key values of a given type through KeyTableAccessor::KeyValues
    // KeyTableAccessor::KeyValues is a map corresponding the const char * key to a OwnedValue_t
    template <typename T>
    class KeyTableAccessor; /*Dont define anything to avoid implicit template instantiation*/

#define BEGIN_KEY_TABLE(class_name)                    \
    template <>                                        \
    class KeyTableAccessor<class_name>                 \
    {                                                  \
        typedef class_name KeyTableAccessor_ClassName; \
                                                       \
    public:                                            \
        inline static KeyValueMap_t KeyValues = {

#define DEFINE_KEY_VALUE(type, name) {#name, OwnedValue_t{(ValueType_t)ValueTypeDeducer<type>::VALUE_TYPE, OffsetOf(&KeyTableAccessor_ClassName::name), sizeof(type)}},

#define END_KEY_TABLE };};

    struct Value_t {
        ValueType_t Type;
        char* Value;
    };

    #define Type(t) case type_##t:return ToString<t>(to, *(t*)from.Value);
    template<> inline
    size_t ToString<Value_t>(char* to, const Value_t& from) {
        switch(from.Type) {
            TYPE_LIST
            default:
                Console::LogError("ToString<Value_t> : Could not convert value !");
                return 0;
        }
    }
    #undef Type

    Value_t FromOwnedValue(void *owner, const OwnedValue_t &owned);


    template<typename TTo>
    TTo FromValue(const Value_t& value) {
        if(value.Type != (ValueType_t)ValueTypeDeducer<TTo>::VALUE_TYPE) {
            Console::LogError("FromValue : Tried to convert keyvalue to the wrong type !");
            return TTo{};
        }
        return *(TTo*)value.Value;
    };


    template <typename TOwner>
    bool GetKeyvalue(Value_t &outValue, TOwner *owner, const char *key) {
        auto owned_iterator = KeyTableAccessor<TOwner>::KeyValues.find(key);
        if(owned_iterator == KeyTableAccessor<TOwner>::KeyValues.end()) {
            return false;
        }
        OwnedValue_t owned = owned_iterator->second;

        outValue = FromOwnedValue(owner, owned);
        return true;
    };

#define Type(t)                                                        \
    case type_##t:                                                     \
    {                                                                  \
        std::pair<int, t> from_string_res = FromString<t>(args, argC); \
        if (from_string_res.first == FROM_STRING_SUCCESS)              \
        {                                                              \
            t val = from_string_res.second;                            \
            memcpy((char *)owner + owned.Offset, &val, owned.Size);    \
        }                                                              \
        return true;                                                   \
    }

    template<typename TOwner>
    inline bool SetKeyvalueFromString(TOwner *owner, const char *key, const char **args, int argC) {
        auto owned_iterator = KeyTableAccessor<TOwner>::KeyValues.find(key);
        if (owned_iterator == KeyTableAccessor<TOwner>::KeyValues.end()) {
            return false;
        }

        OwnedValue_t owned = owned_iterator->second;
        switch(owned.Type) {
            TYPE_LIST
            default:
                return true;
        }
    }

#undef Type

#undef TYPE_LIST

}

#endif