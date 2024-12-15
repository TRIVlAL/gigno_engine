
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
}