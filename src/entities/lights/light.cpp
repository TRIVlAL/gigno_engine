#include "light.h"
#include "../../application.h"

namespace gigno {

    ENTITY_DEFINITIONS(Light, Entity)

    Light::Light() : Entity() {
        if(GetApp() && GetApp()->GetRenderer()) {
            GetApp()->GetRenderer()->SubscribeLightEntity(this);
        }
    }

    Light::~Light() {
        if (GetApp() && GetApp()->GetRenderer()) {
            GetApp()->GetRenderer()->UnsubscribeLightEntity(this);
        }
    }
}