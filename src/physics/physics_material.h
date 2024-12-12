#ifndef PHYSICS_MATERIAL_H
#define PHYSICS_MATERIAL_H

namespace gigno {

    enum PhysicsMaterial_t {
        MAT_CONCRETE,
        MAT_STEEL,
        MAT_PLASTIC
    };

    float GetDynamicFrictionCoefficient(PhysicsMaterial_t a, PhysicsMaterial_t b);
    float GetBounciness(PhysicsMaterial_t mat);

}

#endif