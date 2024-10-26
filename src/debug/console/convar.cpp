#include "convar.h"

#if USE_CONSOLE
#include "../../application.h"

namespace gigno {
    ConVar *ConVar::s_pConVars = nullptr;

    DEFINE_CON_VAR_SETTER(int) {
        if(args.GetArgC() == 0) {
            PrintInfo((ConsoleMessageFlags_t)0);
        } else {
            if(!args.GetArgInt(0, m_Value)) {
                Application::Singleton()->Debug()->GetConsole()->LogInfo("Could not set convar '%s' to '%s' : '%s' is not trivially convertible to integer.", GetName(), args.GetArg(0), args.GetArg(0));
            }
        }
    }

    DEFINE_CON_VAR_PRINT_INFO(int) {
        Application::Singleton()->Debug()->GetConsole()->LogInfo((ConsoleMessageFlags_t)flags, "'%s' = '%d'", GetName(), m_Value);
    }

    DEFINE_CON_VAR_SETTER(float) {
        if(args.GetArgC() == 0) {
            PrintInfo((ConsoleMessageFlags_t)0);
        } else {
            if(!args.GetArgFloat(0, m_Value)) {
                Application::Singleton()->Debug()->GetConsole()->LogInfo("Could not set convar '%s' to '%s' : '%s' is not trivially convertible to float.", GetName(), args.GetArg(0), args.GetArg(0));
            }
        }
    }

    DEFINE_CON_VAR_PRINT_INFO(float) {
        Application::Singleton()->Debug()->GetConsole()->LogInfo((ConsoleMessageFlags_t)flags, "'%s' = '%f'", GetName(), m_Value);
    }

    DEFINE_CON_VAR_SETTER(uint) {
        if(args.GetArgC() == 0) {
            PrintInfo((ConsoleMessageFlags_t)0);
        } else {
            int value{};
            if(!args.GetArgInt(0, value)) {
                Application::Singleton()->Debug()->GetConsole()->LogInfo("Could not set convar '%s' to '%s' : '%s' is not trivially convertible to integer.", GetName(), args.GetArg(0), args.GetArg(0));
                return;
            }
            if(value < 0) {
                Application::Singleton()->Debug()->GetConsole()->LogInfo("Could not set convar '%s' to '%s' : '%s' is negative and '%s' is a positive integer.", GetName(), args.GetArg(0), args.GetArg(0), GetName());
                return;
            }
            m_Value = (unsigned int)value;
        }
    }

    DEFINE_CON_VAR_PRINT_INFO(uint) {
        Application::Singleton()->Debug()->GetConsole()->LogInfo((ConsoleMessageFlags_t)flags, "'%s' = '%u'", GetName(), m_Value);
    }

    DEFINE_CON_VAR_SETTER(vec3) {
        if (args.GetArgC() == 0) {
            PrintInfo((ConsoleMessageFlags_t)0);
        }
        else {
            if(args.GetArgC() < 3) {
                Application::Singleton()->Debug()->GetConsole()->LogInfo("vec3 '%s' requires at least 3 arguments to set!", GetName());
                return;
            }
            float x{};
            float y{};
            float z{};
            bool failed = false;
            if (!args.GetArgFloat(0, x))
            {
                Application::Singleton()->Debug()->GetConsole()->LogInfo("Cannot convert first x component '%s' to float !", args.GetArg(0));
                failed = true;
            }
            if (!args.GetArgFloat(1, y))
            {
                Application::Singleton()->Debug()->GetConsole()->LogInfo("Cannot convert second y component '%s' to float !", args.GetArg(1));
                failed = true;
            }
            if (!args.GetArgFloat(2, z))
            {
                Application::Singleton()->Debug()->GetConsole()->LogInfo("Cannot convert thrid z component '%s' to float !", args.GetArg(2));
                failed = true;
            }
            if(!failed) {
                m_Value = glm::vec3{x, y, z};
            }
        }
    }

    DEFINE_CON_VAR_PRINT_INFO(vec3) {
        Application::Singleton()->Debug()->GetConsole()->LogInfo((ConsoleMessageFlags_t)flags, "'%s' = '(%f, %f, %f)'", GetName(), m_Value.x, m_Value.y, m_Value.z);
    }

    str_ConVar::str_ConVar(const char *name, const char *helpstr, const char *value) : 
        ConVar(name, helpstr), m_DefaultValue{value} {
            size_t size = strlen(value) + 1;
        m_Value = new char[size];
        strcpy(m_Value, value);
    }

    void str_ConVar::Reset() {
        delete[] m_Value;
        size_t size = strlen(m_DefaultValue) + 1;
        m_Value = new char[size];
        strcpy(m_Value, m_DefaultValue);
    }

    DEFINE_CON_VAR_SETTER(str) {
        if(args.GetArgC() == 0) {
            PrintInfo((ConsoleMessageFlags_t)0);
        } else {
        delete[] m_Value;
        size_t size = strlen(args.GetArg(0)) + 1;
        m_Value = new char[size];
        strcpy(m_Value, args.GetArg(0));
        }
    }

    DEFINE_CON_VAR_PRINT_INFO(str) {
        Application::Singleton()->Debug()->GetConsole()->LogInfo((ConsoleMessageFlags_t)flags, "'%s' = '%s'", GetName(), m_Value);
    }

}
#endif