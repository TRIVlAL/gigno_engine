#include "world_spawn.h"
#include <filesystem>

#include "../application.h"

#include "rendered_entity.h"
#include "../physics/rigid_body.h"

namespace gigno {

    ENTITY_DEFINITIONS(WorldSpawn, Entity);

    void WorldSpawn::Init()
    {

        
        {
            std::filesystem::path collisions_path{"assets/maps/" + GetApp()->GetNextMap() + "/collisions/"};
            if(!std::filesystem::exists(collisions_path)) {
                Console::LogWarning("WorldSpawn : Collision Directory '%s' does not exist !", collisions_path.c_str());
            } else {
                for(const std::filesystem::path &model : std::filesystem::directory_iterator(collisions_path)) {
                    if(model.extension() == std::filesystem::path(".obj")) {
                        
                        RigidBody *rb = GetApp()->GetEntityServer()->CreateEntity<RigidBody>();
                        rb->CollisionModelPath = s_ModelPaths.PushWord(model.string().c_str());
                        rb->ModelPath = rb->CollisionModelPath;
                        rb->IsStatic = true;
                        rb->DoRender = false;
                        rb->ColliderType = COLLIDER_HULL;

                        rb->Init();
                    }
                }

            }
        }
        
        {
            std::filesystem::path visuals_path = {"assets/maps/" + GetApp()->GetNextMap() + "/visuals/"};
            if(!std::filesystem::exists(visuals_path)) {
                Console::LogWarning("WorldSpawn : Visuals Directory '%s' does not exist !", visuals_path.string().c_str());
            } else {

                for(std::filesystem::path model : std::filesystem::directory_iterator(visuals_path)) {
                    if(model.extension() == std::filesystem::path(".obj")) {
                        
                        RenderedEntity *re = GetApp()->GetEntityServer()->CreateEntity<RenderedEntity>();

                        re->ModelPath = s_ModelPaths.PushWord(model.string().c_str());

                        re->Scale = glm::vec3{VisualsScale};

                        re->Init();
                    }
                }

            }
        }
        

        Entity::Init();
    }

    void WorldSpawn::CleanUp() {
        Entity::CleanUp();
    }
}