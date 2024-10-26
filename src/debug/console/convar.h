#ifndef CONVAR_H
#define CONVAR_H

#include "command_token.h"
#include <stdio.h>
#include "glm/glm.hpp"
#include "../../features_usage.h"
#include <cstring>

namespace gigno {
#if USE_CONSOLE
    #define CONVAR(type, name, value, ...)\
    static type##_ConVar convar_##name(#name, "", value, ##__VA_ARGS__)
#else
    #define CONVAR(type, name, value, ...)\
    static type##_ConVar convar_##name(value)
#endif

#if USE_CONSOLE
    #define CONVAR_HELP(type, name, helpstr, value, ...)\
    static type##_ConVar convar_##name(#name, helpstr, value, ##__VA_ARGS__)
#else
    #define CONVAR_HELP(type, name, helpstr, value, ...)\
    static type##_ConVar convar_##name(value)
#endif

#if USE_CONSOLE

    class ConVar {
    public:
        static ConVar* s_pConVars;

        ConVar() = delete;
        ConVar(const char *name, const char *helpstr) :
            m_Name{name}, m_HelpString{helpstr} {
                m_pNext = s_pConVars;
                s_pConVars = this;
        }

        virtual void Set(const CommandToken_t &args) = 0;
        virtual void Reset() = 0;
        
        virtual void PrintInfo(int flags) = 0;

        const char *GetHelpString() { return m_HelpString; }

        ConVar* GetNext() { return m_pNext; }
        const char *GetName() { return m_Name; }
    private:

        ConVar* m_pNext;
        const char *m_Name;
        const char *m_HelpString;
    };

#define DEFINE_CON_VAR_TYPE(type)                                                                                                                \
    class type##_ConVar : ConVar                                                                                                                 \
    {                                                                                                                                            \
    public:                                                                                                                                      \
        type##_ConVar() = delete;                                                                                                                \
        type##_ConVar(const char *name, const char *helpstr, const type value) : ConVar(name, helpstr), m_Value{value}, m_DefaultValue{value} {} \
                                                                                                                                                 \
        virtual void Set(const CommandToken_t &args) override;                                                                                   \
        virtual void Reset() override { m_Value = m_DefaultValue; }                                                                              \
                                                                                                                                                 \
        virtual void PrintInfo(int flags) override;                                                                         \
                                                                                                                                                 \
        type Get() { return m_Value; }                                                                                                           \
                                                                                                                                                 \
    private:                                                                                                                                     \
        type m_Value;                                                                                                                            \
        const type m_DefaultValue;                                                                                                               \
    }

#define DEFINE_CON_VAR_TYPE_NAME(type, name)                                                                                                     \
    class name##_ConVar : ConVar                                                                                                                 \
    {                                                                                                                                            \
    public:                                                                                                                                      \
        name##_ConVar() = delete;                                                                                                                \
        name##_ConVar(const char *name, const char *helpstr, const type value) : ConVar(name, helpstr), m_Value{value}, m_DefaultValue{value} {} \
                                                                                                                                                 \
        virtual void Set(const CommandToken_t &args) override;                                                                                   \
        virtual void Reset() override { m_Value = m_DefaultValue; }                                                                              \
                                                                                                                                                 \
        virtual void PrintInfo(int flags) override;                                                                        \
                                                                                                                                                 \
        type Get() { return m_Value; }                                                                                                           \
                                                                                                                                                 \
    private:                                                                                                                                     \
        type m_Value;                                                                                                                            \
        const type m_DefaultValue;                                                                                                               \
    }

#define DEFINE_CON_VAR_SETTER(type)                    \
    void type##_ConVar::Set(const CommandToken_t &args)

#define DEFINE_CON_VAR_PRINT_INFO(type)\
    void type##_ConVar::PrintInfo(int flags)



#else //USE_CONSOLE
#define DEFINE_CON_VAR_TYPE(type) \
    class type##_ConVar {\
    public:\
        type##_ConVar(type value) : m_Value{value} {}\
        type Get() { return m_Value; }\
    private:\
        const type m_Value;\
    }\

#define DEFINE_CON_VAR_TYPE_NAME(type, name) \
    class name##_ConVar {\
    public:\
        name##_ConVar(type value) : m_Value{value} {}\
        type Get() { return m_Value; }\
    private:\
        type m_Value;\
    }\

#endif //USE_CONSOLE

// BASE CONVAR TYPES:

    DEFINE_CON_VAR_TYPE(int);

    DEFINE_CON_VAR_TYPE(float);

    DEFINE_CON_VAR_TYPE_NAME(unsigned int, uint);

    DEFINE_CON_VAR_TYPE_NAME(glm::vec3, vec3);

#if USE_CONSOLE
    // The str_ConVar (i.e char *) is a bit more complex than the other ones. We can not use the macro there.
    class str_ConVar : ConVar {
    public:
        str_ConVar() = delete;
        str_ConVar(const char *name, const char *helpstr, const char * value);
        ~str_ConVar() {
            delete[] m_Value;
        }

        virtual void Set(const CommandToken_t &args) override;
        virtual void Reset() override;

        virtual void PrintInfo(int flags) override;

        char * Get() { return m_Value; } 

    private:
        char *m_Value{};
        const char *m_DefaultValue;
    };
#else
    class str_ConVar {
    public:
        str_ConVar(const char *value) {
            size_t size = strlen(value) + 1;
            m_Value = new char[size];
            strcpy(m_Value, value);
        }
        ~str_ConVar() {
            delete[] m_Value;
        }
        char *Get() { return m_Value; }
    private:
        char *m_Value;
    };
#endif

}

#endif