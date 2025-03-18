#include "light.h"
#include "directional_light.h"
#include "../../application.h"

namespace gigno {

    ENTITY_DEFINITIONS(Light, Entity)
    
    void Light::Init() {
        GetApp()->GetRenderer()->SubscribeLightEntity(this);
        Entity::Init();
    }

    void Light::CleanUp() {
        GetApp()->GetRenderer()->UnsubscribeLightEntity(this);
        Entity::CleanUp();
    }
}