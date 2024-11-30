#include "../command.h"
#include <map>
#include "../../../input/keys.h"
#include "../../../application.h"
#include "../console.h"

namespace gigno {

    std::vector<std::pair<Key_t, char *>> s_BoundCommands{};

    CONSOLE_COMMAND_HELP_UPDATE(bind, "Usage : 'bind [KEY] [command] [?arg1] [?arg2] ...'"
                        "from now-on until program termination, the command will be called with these arguments every time the KEY is pressed") {
        if(args.GetArgC() < 2) {
            Console::LogInfo("'bind' requires at lest 2 arguments !");
            return;
        }
        const char *key_str = args.GetArg(0);
        std::pair<int, Key_t> key_res = FromString<Key_t>(&key_str, 1);
        if(key_res.first != FROM_STRING_SUCCESS) {
            Console::LogInfo("'%s' is not a key !", key_str);
            return;
        }
        Key_t key = key_res.second;
        size_t size = 0;
        for(int i = 1; i < args.GetArgC(); i++) {
            size += strlen(args.GetArg(i)) + 1;
        }
        std::pair<Key_t, char *> &new_com = s_BoundCommands.emplace_back(std::pair(key, new char[size]));
        size_t curr = 0;
        for (int i = 1; i < args.GetArgC(); i++) {
            strcpy(new_com.second + curr*sizeof(char), args.GetArg(i));
            curr += strlen(args.GetArg(i));
            new_com.second[curr] = ' ';
            curr++;
        }
        new_com.second[curr] = '\0';

        Console::LogInfo("Command call '%s' bound to key '%s'", new_com.second, key_str);
    }

    void bind_update(float dt) {
        InputServer *input = Application::Singleton()->GetInputServer();
        for(std::pair<Key_t, char *>& bound : s_BoundCommands) {
            if(input->GetKeyDown(bound.first)) {
                Console::Singleton()->CallCommand(bound.second);
            }
        }
    }

    CONSOLE_COMMAND_HELP(unbind, "Usage : bind [KEY] . The key passed in will no longer have any command bound to") {
        if(args.GetArgC() < 1) {
            Console::LogInfo("unbind requires one argument.");
            return;
        }

        const char *key_str = args.GetArg(0);
        std::pair<int, Key_t> key_res = FromString<Key_t>(& key_str, 1);
        if(key_res.first != FROM_STRING_SUCCESS) {
            Console::LogInfo("Could not convers '%s' to a key.", args.GetArg(0));
            return;
        }

        int i = 0;
        int erased_count = 0;
        while(i < s_BoundCommands.size()) {
            if(s_BoundCommands[i].first == key_res.second) {
                s_BoundCommands.erase(s_BoundCommands.begin() + i);
                erased_count++;
            } else {
                i++;
            }
        }
        Console::LogInfo("Erased %d bindings from the key '%s'", erased_count, args.GetArg(0));
    }
}

