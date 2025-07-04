
#include "../../entities/entity.h"
#include "../../debug/console/command.h"
#include <fstream>

namespace gigno {

    namespace giap {

        const char *ValueTypeToTrenchbroomType(ValueType_t vt) {
            switch(vt) {
                case type_int: return "integer";
                case type_float : return "float";
                case type_cstr : return "string";
                case type_bool : return "boolean";
                case type_vec3 : return "string";
                case type_mat3 : return "string";
                case type_ColliderType_t : return "string";
                default : return "string";
            }
        }

        bool ShouldValueTypeGetQuotes(ValueType_t vt) {
            return !(strcmp(ValueTypeToTrenchbroomType(vt), "integer") == 0);
        }

        CONSOLE_COMMAND_HELP(giap_write_fgd, "Outputs gigno.fgd in the working directory. fgd files are used by trenchbroom to know what entities exist in the codebase") {

            std::ofstream output{"gigno.fgd"};


            for(auto entity : Entity::s_NewEntityMethodMap) {
                if(strcmp(entity.first, "WorldSpawn") == 0) {
                    continue;
                }

                output << "@PointClass = " << entity.first << " : \"\"\n[\n";

                const char *name = entity.first;
                Entity *ent = (entity.second)();
                for(std::pair<const char *, Value_t> keyval : ent->KeyValues()) {

                    if(strcmp(keyval.first, "Position") == 0) {
                        continue;
                    }

                    char *to_string;
                    if(keyval.second.Type != type_vec3 && keyval.second.Type != type_mat3) {
                        size_t size = ToString(nullptr, keyval.second);
                        to_string = new char[size];
                        ToString(to_string, keyval.second);
                    } else {
                        to_string = new char[1];
                        *to_string = '\0';
                    }

                    bool quote = ShouldValueTypeGetQuotes(keyval.second.Type);

                    output << "\t" << keyval.first << "(" << ValueTypeToTrenchbroomType(keyval.second.Type) << ") : \"\" : " << (quote ? "\"" : "") << to_string << (quote ? "\"" : "") << "\n";

                    delete[] to_string;
                }

                output << "]\n\n";

                delete ent;
            }

            Console::LogInfo("Output 'gigno.fgd'");

        }

    }

}

