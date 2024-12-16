#ifndef KEYVALUES_H
#define KEYVALUES_H

#include <cstddef>
#include <map>
#include <any>
#include <cstring>
#include "../../debug/console/console.h"
#include "../../stringify.h"
#include "glm/glm.hpp"
#include "../../algorithm/offset_of.h"

namespace gigno {
    typedef glm::vec3 vec3;
    struct Transform_t;
    typedef const char * cstr;

    //List here every types avaliable for a keyvalue
    #define TYPE_LIST\
    Type(int)\
    Type(float)\
    Type(bool)\
    Type(vec3)\
    Type(cstr)


    #define Type(t) type_##t,
    enum ValueType_t : int{
        TYPE_LIST
    };
    #undef Type


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

        size_t Offset;

        size_t Size;
    };


    struct CompareCharPtr_Func { bool operator()(const char *a, const char *b) const {return strcmp(a, b) < 0;}};

    typedef std::map<const char *, OwnedValue_t, CompareCharPtr_Func> KeyValueMap_t;
    
    template <typename T>
    class KeyTableAccessor;
    /*{
        public: static KeyValueMap_t KeyValues;
    };*/

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
    Value_t GetKeyvalue(TOwner *owner, const char *key) {
        auto& a = KeyTableAccessor<TOwner>::KeyValues;
        auto owned_iterator = KeyTableAccessor<TOwner>::KeyValues.find(key);
        if(owned_iterator == KeyTableAccessor<TOwner>::KeyValues.end()) {
            Console::LogError("GetKeyValue : No value with key '%s' exists !", key);
            return Value_t{};
        }
        OwnedValue_t owned = owned_iterator->second;

        return FromOwnedValue(owner, owned);
    };

}

#endif