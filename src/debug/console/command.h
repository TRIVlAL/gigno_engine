#ifndef COMMAND_H
#define COMMAND_H

#include "../../features_usage.h"

#include "stringify.h"

#if USE_CONSOLE

#include "console.h"

namespace gigno {

    class Command {
    public:
        typedef void(CommandCallback_t)(const CommandToken_t& );
        typedef void(CommandUpdate_t)(float);

        Command() = delete;
        Command(const char *name, CommandCallback_t callback, CommandUpdate_t update_callback, const char *help_string = "");

        Command *GetNext() { return m_pNext; }

        const char *GetName() { return m_Name; }
        const char *GetHelpString() { return m_HelpString; }

        void Execute(const CommandToken_t &token);

        void Update(float dt);
        
        static Command* s_pCommands;
    private:
        Command* m_pNext{};

        CommandCallback_t *m_Callback{};
        CommandUpdate_t *m_Update{};
        const char *m_Name{};
        const char *m_HelpString{};
    };
}
#endif

#if USE_CONSOLE
    #define CONSOLE_COMMAND(name)                \
    static void name(const CommandToken_t &args);\
    static void name##_update(float dt){}\
    static Command Command_##name(#name, name, name##_update);  \
    void name(const CommandToken_t &args) //{ }
#else
    #define CONSOLE_COMMAND(name) /* Console disabled : Console Commands are stripped from this build. Enable them in features_usage.h*/
#endif

#if USE_CONSOLE
    #define CONSOLE_COMMAND_HELP(name, help_string)   \
    static void name(const CommandToken_t &args);\
    static void name##_update(float dt){}\
    static Command Command_##name(#name, name, name##_update, help_string);  \
    void name(const CommandToken_t &args) //{ }
#else
#define CONSOLE_COMMAND_HELP(name, help_string) /* Console diabled : Console Commands are stripped from this build. Enable them in features_usage.h*/
#endif

#if USE_CONSOLE
    #define CONSOLE_COMMAND_HELP_UPDATE(name, help_string)   \
    static void name(const CommandToken_t &args);\
    static void name##_update(float dt); /* Defined by user.*/\
    static Command Command_##name(#name, name, name##_update, help_string);  \
    void name(const CommandToken_t &args) //{ }
#else
#define CONSOLE_COMMAND_HELP_UPDATE(name, help_string) /* Console diabled : Console Commands are stripped from this build. Enable them in features_usage.h*/
#endif

#define COMMAND_ARG_COUNT args.ArgCount

#define COMMAND_REQUIRE_EQU_ARG_COUNT(c) if(args.ArgCount != c) { Console::LogInfo("Command '%s' requires %d arguments",  args.Name, c); return; }
#define COMMAND_REQUIRE_MIN_ARG_COUNT(c) if(args.ArgCount < c) { Console::LogInfo("Command '%s' requires at least %d arguments",  args.Name, c); return; }

#define COMMAND_ARG_I_STR(i, res) char * res = args.Args[i]
#define COMMAND_ARG_0_STR(res) char * res = args.Args[0]
#define COMMAND_ARG_1_STR(res) char * res = args.Args[1]
#define COMMAND_ARG_2_STR(res) char * res = args.Args[1]
#define COMMAND_ARG_3_STR(res) char * res = args.Args[1]
#define COMMAND_ARG_4_STR(res) char * res = args.Args[1]
#define COMMAND_ARG_5_STR(res) char * res = args.Args[1]

#define IMPL_COMMAND_ARG_CONVERT(type, defaultval, argindx, res) type res = defaultval;\
    std::pair<int, type> convert = FromString<type>((const char **)(&args.Args[argindx]), 1);\
    if(convert.first != FROM_STRING_SUCCESS) { Console::LogInfo("Expects " #type " for argument %d. Could not convert %s to "#type, argindx, args.Args[argindx]); return;} else {res = convert.second;}

#define COMMAND_ARG_I_INT(i, res) IMPL_COMMAND_ARG_CONVERT(int, 0, i, res)
#define COMMAND_ARG_0_INT(res) IMPL_COMMAND_ARG_CONVERT(int, 0, 0, res)
#define COMMAND_ARG_1_INT(res) IMPL_COMMAND_ARG_CONVERT(int, 0, 1, res)
#define COMMAND_ARG_2_INT(res) IMPL_COMMAND_ARG_CONVERT(int, 0, 2, res)
#define COMMAND_ARG_3_INT(res) IMPL_COMMAND_ARG_CONVERT(int, 0, 3, res)
#define COMMAND_ARG_4_INT(res) IMPL_COMMAND_ARG_CONVERT(int, 0, 4, res)
#define COMMAND_ARG_5_INT(res) IMPL_COMMAND_ARG_CONVERT(int, 0, 5, res)

#define COMMAND_ARG_I_FLOAT(i, res) IMPL_COMMAND_ARG_CONVERT(float, 0.0f, i, res)
#define COMMAND_ARG_0_FLOAT(res) IMPL_COMMAND_ARG_CONVERT(float, 0.0f, 0, res)
#define COMMAND_ARG_1_FLOAT(res) IMPL_COMMAND_ARG_CONVERT(float, 0.0f, 1, res)
#define COMMAND_ARG_2_FLOAT(res) IMPL_COMMAND_ARG_CONVERT(float, 0.0f, 2, res)
#define COMMAND_ARG_3_FLOAT(res) IMPL_COMMAND_ARG_CONVERT(float, 0.0f, 3, res)
#define COMMAND_ARG_4_FLOAT(res) IMPL_COMMAND_ARG_CONVERT(float, 0.0f, 4, res)
#define COMMAND_ARG_5_FLOAT(res) IMPL_COMMAND_ARG_CONVERT(float, 0.0f, 5, res)

#define COMMAND_ARG_I_KEY(i, res) IMPL_COMMAND_ARG_CONVERT(Key_t, KEY_MAX_ENUM, i, res)
#define COMMAND_ARG_0_KEY(res) IMPL_COMMAND_ARG_CONVERT(Key_t, KEY_MAX_ENUM, 0, res)
#define COMMAND_ARG_1_KEY(res) IMPL_COMMAND_ARG_CONVERT(Key_t, KEY_MAX_ENUM, 1, res)
#define COMMAND_ARG_2_KEY(res) IMPL_COMMAND_ARG_CONVERT(Key_t, KEY_MAX_ENUM, 2, res)
#define COMMAND_ARG_3_KEY(res) IMPL_COMMAND_ARG_CONVERT(Key_t, KEY_MAX_ENUM, 3, res)
#define COMMAND_ARG_4_KEY(res) IMPL_COMMAND_ARG_CONVERT(Key_t, KEY_MAX_ENUM, 4, res)
#define COMMAND_ARG_5_KEY(res) IMPL_COMMAND_ARG_CONVERT(Key_t, KEY_MAX_ENUM, 5, res)

#endif