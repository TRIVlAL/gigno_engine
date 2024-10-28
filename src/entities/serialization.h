#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <vector>
#include "string"
#include "../features_usage.h"
#include "../stringify.h"

#if USE_IMGUI
    #include "../rendering/gui.h"
#endif

namespace gigno {
    class Entity;

#define ENABLE_SERIALIZATION(type)\
    public: inline virtual void AddSerializedProperties() override; virtual const char *GetTypeName() const override { return #type; }; private:\


#define DEFINE_SERIALIZATION(type)                          \
    template <>                                             \
    constexpr inline const char *TypeString<type>() { return #type; } \
    void type::AddSerializedProperties()

#define SERIALIZE(type, name)\
    serializedProps.push_back(new SerializedProperty<type>(#name, &name))

#define SERIALIZATION_LINE_SKIP\
    serializedProps.push_back(new EmptySerializedProperty("#LINE_SKIP"))

#define SERIALIZATION_SEPARATOR\
    serializedProps.push_back(new EmptySerializedProperty("#SEPARATOR"))

#define SERIALIZE_BASE_CLASS(base_class)\
    base_class::AddSerializedProperties(); SERIALIZATION_SEPARATOR;

    class BaseSerializedProperty
    {
    public:
        BaseSerializedProperty() = delete;
        BaseSerializedProperty(const char *name, void *data)
            : m_Name{name}, m_Data{data} {}

        virtual size_t ValueToString(char *to) const = 0;
        virtual const char *TypeToString() const = 0;
        const char *GetName() const { return m_Name; }
        void *GetData() const { return m_Data; }
    private:
        const char *m_Name;
        void *m_Data;
    };

    template<typename T>
    class SerializedProperty : public BaseSerializedProperty {
    public:
        SerializedProperty() = delete;
        SerializedProperty(const char* name, T* data) 
            : BaseSerializedProperty(name, static_cast<void *>(data)) {}

        virtual size_t ValueToString(char *to) const override {
            return ToString<T>(to, *(static_cast<T*>(GetData())));
        }
        virtual const char *TypeToString() const override {
            return TypeString<T>();
        }
    };

    class EmptySerializedProperty : public BaseSerializedProperty {
    public:
        EmptySerializedProperty() = delete;
        EmptySerializedProperty(const char *tag) 
            : BaseSerializedProperty(tag, nullptr) {}
        
        virtual size_t ValueToString(char *to) const override {
            return -1;
        }
        virtual const char *TypeToString() const override {
            return "<serialization tag>";
        }
    };
    
    class Serialization { // See top of the file for explanation / How-To.
        Serialization() = delete;
    public:
        static const std::vector<BaseSerializedProperty *>& GetProperties(const Entity *entity);
        static bool IsSpecialToken(BaseSerializedProperty *prop);
#if USE_IMGUI
        static void HandleSpecialTokenForImGui(const BaseSerializedProperty *prop);
#endif
    };

}

#endif