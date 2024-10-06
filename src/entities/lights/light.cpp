#include "light.h"
#include "../../application.h"

namespace gigno {

    Light::Light() : Entity() {
        GetApp()->GetRenderer()->SubscribeLightEntity(this);
    }

    Light::~Light() {
        GetApp()->GetRenderer()->UnsubscribeLightEntity(this);
    }

}