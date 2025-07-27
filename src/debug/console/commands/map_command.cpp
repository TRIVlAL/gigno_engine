#include "../command.h"
#include "../../application.h"
#include <filesystem>
#include "../convar.h"
#include <string>
#include "features_usage.h"

namespace gigno {

    #if USE_CONSOLE

    CONSOLE_COMMAND_HELP(map, "Loads the map file passed as argument. If no arguments given, lists the avaliable map files.") {

        if(COMMAND_ARG_COUNT == 0) {
            std::filesystem::path path{"assets/maps/"};
            Console::LogInfo("Available maps :");
            for (auto &p : std::filesystem::directory_iterator(path)) {
                if (p.is_directory()) {
                    Console::LogInfo(MESSAGE_NO_TIME_CODE_BIT, "     - %s", p.path().stem().string().c_str());
                }
            }

            return;
        }

        COMMAND_REQUIRE_EQU_ARG_COUNT(1);

        COMMAND_ARG_0_STR(map_name);
        
        Application::Singleton()->LoadMap(map_name);
    }

    #endif

}