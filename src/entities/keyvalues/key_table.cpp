
#include "key_table.h"
#include "../../debug/console/console.h"
#include "../entity.h"

namespace gigno {
    
    Value_t FromOwnedValue(void *owner, const OwnedValue_t &owned) {
        Value_t value{};
        value.Type = owned.Type;
        value.Value = new char[owned.Size];
        memcpy(value.Value, (char *)owner + owned.Offset, owned.Size);
        return value;
    }

    void Work() {
        A a{};
        a.i = 5;
        a.j = 2.2f;
        a.k = 2;
        a.v = glm::vec3{0.0f, 3.0f, 2.0f};
        glm::vec3 res = FromValue<glm::vec3>(GetKeyvalue(&a, "v"));
        Console::LogInfo("Keyvalue test : %d %f %d (%f %f %f)", FromValue<int>(GetKeyvalue(&a, "i")), FromValue<float>(GetKeyvalue(&a, "j")), FromValue<int>(GetKeyvalue(&a, "k")),
                            res.x, res.y, res.z);

        Entity e;
        e.Position = glm::vec3{0.0f, 1.0f, 2.0f};
        res = FromValue<glm::vec3>(GetKeyvalue(&e, "Position"));
        Console::LogInfo("Keyvalue test entity : (%f %f %f)", res.x, res.y, res.z);
    };
}