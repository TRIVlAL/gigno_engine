#include "serialization.h"
#include "entity.h"

namespace gigno {

    bool PropertySerializationData_t::ToString(std::string &to)
    {
        if (Type == "int")
        {
            to = std::to_string(*((int *)Value));
            return true;
        }
        else if (Type == "float")
        {
            to = std::to_string(*((float *)Value));
            return true;
        }
        else if (Type, "glm::vec3")
        {
            glm::vec3 vec = *((glm::vec3 *)Value);
            to = "(" + std::to_string(vec.x) + "," + std::to_string(vec.y) + "," + std::to_string(vec.z) + ")";
            return true;
        }
        return false;
    }

    std::vector<PropertySerializationData_t> Serialization::GetProperties(const Entity *entity) {
        return entity->serializedProps;
    }

    bool Serialization::IsSpecialToken(const PropertySerializationData_t &prop) {
        return prop.Name[0] == '#'; 
    }

#if USE_IMGUI
    void Serialization::HandleSpecialTokenForImGui(const PropertySerializationData_t &prop) {
        if(prop.Name == "#LINE_SKIP") {
            ImGui::NewLine();
        } else if(prop.Name == "#SEPARATOR") {
            ImGui::Separator();
        }
    }
#endif

}