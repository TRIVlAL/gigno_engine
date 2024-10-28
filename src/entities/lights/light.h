#ifndef LIGHT_H
#define LIGHT_H

#include "../entity.h"

namespace gigno {

    const float LIGHT_DATA_DIRECTIONAL = 1.0f;
    const float LIGHT_DATA_POINT = 2.0f;
    const float LIGHT_DATA_ENVIRONMENT = 3.0f;

    class Light : public Entity {
        ENABLE_SERIALIZATION(Light);
    public:
        Light();
        ~Light();

        virtual uint32_t DataSlotsCount() const = 0;
        virtual void FillDataSlots(glm::vec4 *data) const = 0;

    private:
    };

    DEFINE_SERIALIZATION(Light) {
        SERIALIZE_BASE_CLASS(Entity);
    }
}

#endif