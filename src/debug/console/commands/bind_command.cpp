#include "../command.h"
#include <map>
#include "../../../input/keys.h"
#include "../../../application.h"
#include "../console.h"

namespace gigno {

    #if USE_CONSOLE

    std::vector<std::pair<Key_t, CommandToken_t>> s_BoundCommands{};

    CONSOLE_COMMAND_HELP_UPDATE(bind, "Usage : 'bind [KEY] [command] [?arg1] [?arg2] ...'"
                        "from now-on until program termination, the command will be called with these arguments every time the KEY is pressed") {

        COMMAND_REQUIRE_MIN_ARG_COUNT(2);

        COMMAND_ARG_0_KEY(key);
        
        size_t size = 0;

        for(int i = 1; i < COMMAND_ARG_COUNT; i++) {
            size += strlen(args.Args[i]) + 1;
        }

        CommandToken_t new_tokens{};
        strcpy_s(new_tokens.Data, args.Data);

        size_t indx = (args.Args[2] - args.Data);
        new_tokens.Name = new_tokens.Data + indx;

        for(size_t i = 2; i < CONSOLE_COMMAND_MAX_ARG_COUNT; i++) {

            size_t indx = (args.Args[i] - args.Data);
            new_tokens.Args[i - 2] = new_tokens.Data + indx;
        }

        new_tokens.ArgCount = args.ArgCount - 2;

        s_BoundCommands.emplace_back(std::pair<Key_t, CommandToken_t>{key, new_tokens});


        Console::LogInfo("Command call successfully bound");
    }

    void bind_update(float dt) {
        InputServer *input = Application::Singleton()->GetInputServer();

        for(std::pair<Key_t, CommandToken_t>& bound : s_BoundCommands) {
            if(input->GetKeyDown(bound.first)) {
                Console::Singleton()->CallCommandTokenized(bound.second);
            }
        }
    }

    CONSOLE_COMMAND_HELP(unbind, "Usage : bind [KEY] . The key passed in will no longer have any command bound to") {

        COMMAND_REQUIRE_EQU_ARG_COUNT(1);

        COMMAND_ARG_0_KEY(key);
        COMMAND_ARG_0_STR(key_str);

        int i = 0;
        int erased_count = 0;
        while(i < s_BoundCommands.size()) {
            if(s_BoundCommands[i].first == key) {
                s_BoundCommands.erase(s_BoundCommands.begin() + i);
                erased_count++;
            } else {
                i++;
            }
        }
        
        Console::LogInfo("Erased %d bindings from the key '%s'", erased_count, key_str);
    }

    #endif
}

