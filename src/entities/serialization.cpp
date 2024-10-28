#include "serialization.h"
#include "entity.h"

namespace gigno {

    const std::vector<BaseSerializedProperty *> & Serialization::GetProperties(const Entity *entity) {
        return entity->serializedProps;
    }

    bool Serialization::IsSpecialToken(BaseSerializedProperty *prop) {
        return dynamic_cast<EmptySerializedProperty *>(prop) != nullptr;
    }

#if USE_IMGUI
    void Serialization::HandleSpecialTokenForImGui(const BaseSerializedProperty *prop) {
        if(const EmptySerializedProperty *tag = dynamic_cast<const EmptySerializedProperty *>(prop)) {
            if(strcmp(tag->GetName(), "#LINE_SKIP") == 0) {
                ImGui::NewLine();
            } else if(strcmp(tag->GetName(), "#SEPARATOR") == 0) {
                ImGui::Separator();
            }
        }
    }
#endif

}