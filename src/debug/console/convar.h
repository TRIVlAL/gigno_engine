/*
    CONVAR
A Console Variable (Convar) is a Global variable that can be modified AT RUNTIME from the Console. 
    - The type of the variable must be fully stringify-able (see stringify.h for more info). That means that The following functions
    are implemented for this type : ToString(...), FromString(...) and TypeString()
    - Each Convars must be initialized IN GLOBAL SCOPE !

    - you refer by a convar by its name (given in the constructor) to set it, followed by the necessary arguments.
        - for exemple : an int convar called 'my_test1' is set to 5 by entering into the console 'my_test1 5'
    - If the console is disabled, Convars act as simple constant variables.
    - The convar of type T has an implicit conversion to the type T.
    - T's copy constructor must exist.
*/

#ifndef CONVAR_H
#define CONVAR_H

#include "../../features_usage.h"

#if USE_CONSOLE

#include "command_token.h"
#include "../../stringify.h"


namespace gigno {

    #define CONVAR(type, name, value, help_string) \
        Convar<type> convar_##name = Convar<type>(#name, help_string, value);
    
    class BaseConvar {
    public:
        BaseConvar(const char *name, const char *helpstr) 
            : m_Name{name}, m_HelpString{helpstr} {
            m_pNext = s_pConvars;
            s_pConvars = this;
        }

        virtual void Set(const CommandToken_t &args) = 0;
        virtual size_t ValToString(char *to) const = 0;
        virtual const char *TypeToString() const = 0;

        const char *GetName() const { return m_Name; }
        const char *GetHelpString() const { return m_HelpString; }
        BaseConvar *GetNext() const { return m_pNext; }

        void LogInfo() const;

        void HandleSetResult(const CommandToken_t &token, int result) const;

        inline static BaseConvar *s_pConvars = nullptr;
    private: 
        BaseConvar *m_pNext;

        const char *m_Name;
        const char *m_HelpString;
    };

    /*
    T must be fully stringify-able (see strigify.h)
    */
    template<typename T>
    class Convar : BaseConvar {
    public:
        Convar() = delete;
        Convar(const char *name, const char *helpstr, T val)
            : m_Value{val}, BaseConvar(name, helpstr) {}
        
        virtual void Set(const CommandToken_t &args) override {
            if(args.GetArgC() > 0) {
                std::pair<int, T> result = FromString<T>(args);
                if(result.first) {
                    BaseConvar::HandleSetResult(args, result.first);
                    return;
                }
                m_Value = result.second;
            }

            LogInfo();
        }
        virtual size_t ValToString(char *to) const override {
            return ToString<T>(to, m_Value);
        }
        virtual const char *TypeToString() const override {
            return TypeString<T>();
        }


        operator T() const { return m_Value; }
    private:
        T m_Value{};
    };

}

#else //USE_CONSOLE

#define CONVAR(type, name, value, help_string)\
    /*const*/ type convar_##name = value;

namespace gigno {

    template<typename T>
    class Convar {
    public:
        Convar() = delete;
        Convar(const char *name, const char *helpstr, T val)
            : m_Value{val} {}

        operator T() const { return m_Value; }
    private:
        const T m_Value;
    };

}

#endif


#endif