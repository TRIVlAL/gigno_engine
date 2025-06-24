#ifndef WORLD_SPAWN_H
#define WORLD_SPAWN_H

#include "entity.h"
#include "../algorithm/string_buffer.h"

namespace gigno {

    class WorldSpawn : public Entity {
        ENTITY_DECLARATIONS(WorldSpawn, Entity)
    public:
        
        virtual void Init() override;
        virtual void CleanUp() override;

        cstr CollisionsPath{};
        cstr VisualsPath{};
        float VisualsScale = 0.1f;

    private:
        StringBuffer m_ModelPaths{1028};
    };

    BEGIN_KEY_TABLE(WorldSpawn)
        DEFINE_KEY_VALUE(cstr, CollisionsPath)
        DEFINE_KEY_VALUE(cstr, VisualsPath)
        DEFINE_KEY_VALUE(float, VisualsScale)
    END_KEY_TABLE

}

#endif