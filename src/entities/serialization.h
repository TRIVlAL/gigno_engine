#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <vector>
#include "string"
#include "../core_macros.h"

#if USE_IMGUI
    #include "../rendering/gui.h"
#endif

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

#define SERIALIZATION_LINE_SKIP serializedProps.push_back(PropertySerializationData_t{std::string{"##LINE_SKIP##"}, nullptr, std::string{""}});
#define SERIALIZATION_SEPARATOR serializedProps.push_back(PropertySerializationData_t{std::string{"##SEPARATOR##"}, nullptr, std::string{""}});

namespace gigno {
    class Entity;

    struct PropertySerializationData_t
    {
        const std::string Name;
        const void *Value;
        const std::string Type;

        bool ToString(std::string &to);
    };

    class Serialization {
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