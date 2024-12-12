#include "physics_material.h"
#include "../error_macros.h"

// TODO : should probably find better source instead of scouring the web but it will do for now.

float gigno::GetDynamicFrictionCoefficient(PhysicsMaterial_t a, PhysicsMaterial_t b) {
    PhysicsMaterial_t A = a < b ? a : b;
    PhysicsMaterial_t B = a < b ? b : a;

    if(A == MAT_CONCRETE) {
        switch(B) {
            case MAT_CONCRETE:
                //@ https://www.concreteconstruction.net/how-to/friction-factor-for-concrete-on-concrete_o#:~:text=PCI's%20Design%20Handbook%20says%20the,for%20dry%20conditions%20is%200.80.
                return 0.8f;
            case MAT_STEEL:
                //@ https://www.aisc.org/globalassets/aisc/research-library/shear-transfer-in-exposed-column-base-plates.pdf
                return 0.45f;
            case MAT_PLASTIC:
                //@ https://www.concrete.org.uk/fingertips-nuggets.asp?cmd=display&id=1026#:~:text=Concrete%20%40%20your%20Fingertips&text=Ff%20%3D%20wu%2C%20where%20F,the%20concrete%20and%20sub%2Dbase.
                return 0.2f;
        }
    } else if(A == MAT_STEEL) {
        switch(B) {
            case MAT_STEEL:
                //@ https://www.engineeringtoolbox.com/friction-coefficients-d_778.html
                return 0.42f;
            case MAT_PLASTIC:
                //@ https://avsld.com.sg/coefficient-of-friction/
                return 0.35f;
        }
    }else if(A == MAT_PLASTIC) {
        switch(B){
            case MAT_PLASTIC:
                //@ https://www.tribology-abc.com/abc/cof.htm
                return 0.3f;
        }
    }

    ERR_MSG_V(0.0f, "Querrying Friction COefficient of a Material that does not exist !");
}

float gigno::GetBounciness(PhysicsMaterial_t mat) {
    // Note : these numbers are made up
    // TODO : Find reliable source and define bounciness of every combination
    //        of material like we do with friction coefficient ?
    switch(mat) {
        case MAT_CONCRETE:
            return 0.4f;
        case MAT_PLASTIC:
            return 0.55f;
        case MAT_STEEL:
            return 0.7f;
    }

    ERR_MSG_V(0.0f, "Querrying Bounciness of a Material that does not exist !");
}
