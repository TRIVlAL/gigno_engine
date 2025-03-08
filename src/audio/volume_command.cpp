#include "../debug/console/command.h"
#include "../stringify.h"
#include "../application.h"

namespace gigno {

    CONSOLE_COMMAND_HELP(au_volume, "Pass float. Sets the volume. Default is 1.0") {
        if(args.GetArgC() > 0) {
            const char *a = args.GetArg(0);
            std::pair<int, float> result = FromString<float>(&a, 1);
            if(result.first == FROM_STRING_SUCCESS) {
                if(Application::Singleton()) {
                    Application::Singleton()->GetAudioServer()->SetGlobalVolume(result.second);
                    return;
                }
            }
        }
        Console::LogInfo("au_volume failed.");
    }

}