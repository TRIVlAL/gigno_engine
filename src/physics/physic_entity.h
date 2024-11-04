#ifndef PHYSIC_ENTITY
#define PHYSIC_ENTITY

#include "../entities/rendered_entity.h"

namespace gigno {

    class PhysicEntity : public RenderedEntity {
        ENABLE_SERIALIZATION(PhysicEntity);

    public:
        inline static PhysicEntity *pFirstPhysicEntity = nullptr;

        PhysicEntity(ModelData_t modelData) : RenderedEntity{modelData} {
            pNextPhysicEntity = pFirstPhysicEntity;
            pFirstPhysicEntity = this;
        }
        ~PhysicEntity() {
            PhysicEntity *curr = pFirstPhysicEntity;
            if(curr == this) {
                pFirstPhysicEntity = pNextPhysicEntity;
            }else {
                while(curr) {
                    if(curr->pNextEntity == this) {
                        curr->pNextEntity = pNextEntity;
                    }
                    curr = curr->pNextPhysicEntity;
                }
            }
        }

        virtual void PhysicThink(float dt) = 0;

        // Next rendered entity in the RenderingServer's chain of all rendered entities (linked list). Set on construction.
        // 'nullptr' if is last element.
        PhysicEntity *pNextPhysicEntity{};

    };

    DEFINE_SERIALIZATION(PhysicEntity) {
        SERIALIZE_BASE_CLASS(RenderedEntity);
    }

}

#endif