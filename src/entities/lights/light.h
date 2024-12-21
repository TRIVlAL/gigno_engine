#ifndef LIGHT_H
#define LIGHT_H

#include "../entity.h"

namespace gigno {

    const float LIGHT_DATA_DIRECTIONAL = 1.0f;
    const float LIGHT_DATA_POINT = 2.0f;
    const float LIGHT_DATA_ENVIRONMENT = 3.0f;

    class Light : public Entity {
        ENTITY_DECLARATIONS(Light, Entity)
    public:
        Light();
        ~Light();

        virtual uint32_t DataSlotsCount() const { return 0; };
        virtual void FillDataSlots(glm::vec4 *data) const {return;};

        Light *pNextLight;
    private:
    };

    BEGIN_KEY_TABLE(Light)
    END_KEY_TABLE

}

#endif