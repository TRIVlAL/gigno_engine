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
        Light() : Entity() {};
        ~Light(){};

        virtual void Init() override;
        virtual void CleanUp() override;

        // Returns how many vec4 are required to be passed to the shader.
        virtual uint32_t DataSlotsCount() const { return 0; };
        // Fills the next DataSlotCOunt() elements of the data c-array 
        // with the data that needs to be passed to the shader.
        virtual void FillDataSlots(glm::vec4 *data) const {return;};

        //NOTE : Only Directional lights support shadow mapping.

        Light *pNextLight;
    private:
    };

    BEGIN_KEY_TABLE(Light)
    END_KEY_TABLE

}

#endif