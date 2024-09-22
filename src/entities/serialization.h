#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <vector>
#include "string"
#include "../core_macros.h"

#if USE_IMGUI
    #include "../rendering/gui.h"
#endif

/*
HOW TO USE SERIALIZATION:

Any Entity can serialize any number of properties (variables) so that they can be accessed through the static method Serialization::GetProperties( ... ).
Simply add at the top of the class declaration that derives Entity the following macros.

in between BEGIN_SERIALIZE and END_SERIALIZE, add the macro SERIALIZE for each property you wish to serialize.

Now, any time someone will call GetProperties( ... ) with this entity as a parameter, they will recieve a vector of every properties,
with their name, their type as a string, and a void pointer to their value.

Even if you don't want to Serialize data for this class, you should still add the macros to make sure that data is still serialized 
by the base classes. In that case, simply use ENABLE_SERIALIZE():

    EXAMPLE:

    class Foo : DerivesFromEntity {
        BEGIN_SERIALIZE( Foo, DerivesFromEntity )
        SERIALIZE(int meber_variable)
        END_SERIALIZE

        int member_variable;
        //...
    }

    class Bar : DerivesFromEntity { // Bar does not want to serialize any additional data
        ENABLE_SERIALIZE( Bar, DerivesFromEntity )

        //...
    }

SPECIAL TOKENS:

If you wish to have special layout option, you can use the macros : SERIALIZATION_LINE_SKIP, SERIALIZATION_SEPARATOR.
This will add properties with special Serialization Token as Name. These token always start with a hashtag '#'.
You can check if the property is a special token using Serialization::IsSpecialToken( ... ). These token can be handled
differently by any implementation you need. In the case of a ImGui ui, the method Serialization::HandleSpecialTokenForImGui( ... )
can be used.
*/

#define ENABLE_SERIALIZE(entity_type_name, base_class) BEGIN_SERIALIZE(entity_type_name, base_class) END_SERIALIZE;

#define BEGIN_SERIALIZE(entity_type_name, base_class)                                     \
public:                                                                                   \
    virtual std::string GetTypeName() override { return std::string{#entity_type_name}; } \
                                                                                          \
protected:                                                                                \
    virtual void AddSerializedProperties() override                                       \
    {                                                                                     \
        base_class::AddSerializedProperties();                                            \
        SERIALIZATION_SEPARATOR;

#define END_SERIALIZE \
    }                 \
                      \
private:

#define SERIALIZE(type, name) serializedProps.push_back(PropertySerializationData_t{std::string{#name}, (void *)&name, std::string{#type}});

#define SERIALIZATION_LINE_SKIP serializedProps.push_back(PropertySerializationData_t{std::string{"#LINE_SKIP"}, nullptr, std::string{""}});
#define SERIALIZATION_SEPARATOR serializedProps.push_back(PropertySerializationData_t{std::string{"#SEPARATOR"}, nullptr, std::string{""}});

namespace gigno {
    class Entity;

    struct PropertySerializationData_t
    {
        const std::string Name;
        const void *Value;
        const std::string Type;

        bool ToString(std::string &to);
    };

    
    class Serialization { // See top of the file for explanation / How-To.
        Serialization() = delete;
    public:
        static std::vector<PropertySerializationData_t> GetProperties(const Entity *entity);
        static bool IsSpecialToken(const PropertySerializationData_t &prop);
    #if USE_IMGUI
        static void HandleSpecialTokenForImGui(const PropertySerializationData_t &prop);
    #endif
    };

}

#endif