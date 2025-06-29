#ifndef TARGET_H
#define TARGET_H

#include "entity.h"

namespace gigno {


    //Target is just a valid empty entity.
    class Target : public Entity {

        ENTITY_DECLARATIONS(Target, Entity);

    };

    BEGIN_KEY_TABLE(Target)
    END_KEY_TABLE

}

#endif