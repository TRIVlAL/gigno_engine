#include "../command.h"
#include "../../application.h"
#include <filesystem>
#include "../convar.h"
#include <string>

namespace gigno {

    CONVAR(const char *, map_path, "maps/", "Path from application directory where maps are found by map command. Default is /maps");

    CONSOLE_COMMAND_HELP(map, "Loads the map file passed as argument. If no arguments given, lists the avaliable map files.") {
        if(args.GetArgC() > 0) {
            std::string path {(const char *)convar_map_path};
            path += args.GetArg(0);
            path += ".map";
            Application::Singleton()->LoadMap(path.c_str());
        } else {
            std::filesystem::path path{(const char *)convar_map_path};
            std::filesystem::path extension(".map");
            Console::LogInfo(".map files in directory (%s):", (const char *)convar_map_path);
            for (auto &p : std::filesystem::recursive_directory_iterator(path))
            {
                if (p.path().extension() == extension) {
                    Console::LogInfo("- %s", p.path().stem().string().c_str(), MESSAGE_NO_TIME_CODE_BIT);
                }
            }
        }
    }

}