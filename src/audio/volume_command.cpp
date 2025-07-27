#include "../debug/console/command.h"
#include "../stringify.h"
#include "../application.h"

namespace gigno {

    CONSOLE_COMMAND_HELP(au_volume, "Pass float. Sets the volume. Default is 1.0") {

        COMMAND_REQUIRE_MIN_ARG_COUNT(1);

        COMMAND_ARG_0_FLOAT(new_volume);

        if(Application::Singleton()) {
            Application::Singleton()->GetAudioServer()->SetGlobalVolume(new_volume);
            return;
        }

        Console::LogInfo("au_volume failed.");
    }

}