#ifndef WORLD_SPAWN_H
#define WORLD_SPAWN_H

#include "entity.h"
#include "../utils/string_buffer.h"

namespace gigno {

    class WorldSpawn : public Entity {
        ENTITY_DECLARATIONS(WorldSpawn, Entity)
    public:
        
        virtual void Init() override;
        virtual void CleanUp() override;

        float VisualsScale = 0.1f;

    private:
        // static so it never gets destroyed : it must live for the entire lifetime
        //  of the app as it is then referenced by the physics server
        inline static StringBuffer s_ModelPaths{1028};
    };

    BEGIN_KEY_TABLE(WorldSpawn)
        DEFINE_KEY_VALUE(float, VisualsScale)
    END_KEY_TABLE

}

#endif