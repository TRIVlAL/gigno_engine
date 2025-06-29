
#include <vector>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <chrono>

#include "../../debug/console/command.h"
#include "../../debug/console/console.h"
#include "../../debug/console/convar.h"
#include "../../vendor/glm/glm/glm.hpp"
#include "../../utils/string_buffer.h"

#define ENABLE_GIAP 1

#if ENABLE_GIAP

namespace gigno {

    /*
    -------------------------------------------------------
    GIAP : Gigno Map Generator
    -------------------------------------------------------

    Command-Line tool in the Gigno Console.

    Converts Quake-style .map files (specificaly Trenchbroom's) to a set of files usable by the engine !
    
    Usage : 
        giap_compile            ----> Lists the Trenchbroom maps (.map) in the 'mapping/' directory
        giap_compile [NAME]     ----> Finds the 'NAME.map' trenchbroom map file in 'mapping/
                                      Outputs the files usable by the engine within the 'assets/maps/' diretory 

        CAVEAT : currently, one manual step is still necessary after compiling. use trenchbroom's export option to 'export as Wavefront .obj'
        and place the generated model in 'assets/maps/NAME/visuals/'. If nothing is shown when you load the map, but the collisions are working, 
        you are probably missing that step.

        giap_compile_all        ----> call giap_compile with every Trenchbroom maps in the 'mapping/' directory
    */

    namespace giap {

        struct Plane_t {
            //planes of equation : normal dot (x, y, z) = c

            glm::vec3 Normal{}; //normalized
            float C{};
        };

        struct Brush_t {
            std::vector<Plane_t> Planes{};
        };

        // Representation inbetween a Brush and a Mesh :
        // All the vertices are in place, as well as on what plane they're on.
        // Only thing missing : the triangulation of those vertices.
        struct DottedBrush_t {
            const Brush_t *Original{};

            std::vector<glm::vec3> Vertices{}; //list of every vertices composing the brush

            std::vector<std::vector<int>> PerPlaneVertices{}; // for every plane in the original brush, a vector of indices within the vertices array
        };

        struct Mesh_t {
            int VerticesCount{};
            const glm::vec3 *Vertices{};
        
            std::vector<glm::vec3> Normals{};
        
            struct ObjVert_t {
                int PosIndex{};
                int NormalIndex{};
            };

            std::vector<ObjVert_t> Indices{};
        };

        struct Entity_t {
            const char *Name{};
            std::vector<const char *> Keys{}; //interlieved key-value
            std::vector<size_t> KeysValueCount{}; //interlieved key-value
            std::vector<const char *> Values{}; //interlieved key-value
        };

        void GiapCompileImpl(const char *map_name);

        void ParseTrenchbroomMap(std::ifstream &file, std::vector<Brush_t> &outBrushes, std::vector<Entity_t> &outEntities, StringBuffer &keyValuesStringBuffer);
        void BrushesToDottedBrushes(const std::vector<Brush_t> &brushes, std::vector<DottedBrush_t> &outDotteds);
        //Careful !! Will mutate 'dotteds'.
        void DottedBrushesToMeshes(const std::vector<DottedBrush_t> &dotteds, std::vector<Mesh_t> &outMeshes);

        void WriteMeshToObj(const Mesh_t &mesh, std::ofstream &output);
        void WriteGMAPFile(const char *mapName, const std::vector<Entity_t> &entities, std::ofstream &output);

        CONVAR(float, giap_scale, 0.035f, "Compiled map will be scaled by that amount");
        CONVAR(const char *, giap_map_input_directory, "../mapping/", "path in which the trenchbroom maps are stored");
        
        const char *GIAP_MAP_OUTPUT_DIRECTORIES[2] = {"../assets/maps/", "assets/maps"};

        CONSOLE_COMMAND_HELP(giap_compile, "Compiles the trenchbroom map given as argument into a map usable by the engine."
                                            "If no arguments are give, lists the available trenchbroom map") {
            
            if(args.GetArgC() < 1) {
                if(!std::filesystem::exists((std::string)convar_giap_map_input_directory)) {
                    Console::LogWarning("GIAP : Map Input Directory '%s' does not exist !", ((std::string)convar_giap_map_input_directory).c_str());
                    return;
                }

                Console::LogInfo("Available trenchbroom maps :");

                for(auto file : std::filesystem::directory_iterator((std::string)convar_giap_map_input_directory)) {
                    if(file.path().extension() == std::filesystem::path(".map")) {
                        Console::LogInfo((MESSAGE_NO_TIME_CODE_BIT), "     - %s", file.path().stem().string().c_str());
                    }
                }
                return;
            }

            const char *map_name = args.GetArg(0);

            GiapCompileImpl(map_name);
        }
        
        CONSOLE_COMMAND_HELP(giap_compile_all, "Compiles every trenchbroom maps.") {

            if(!std::filesystem::exists((std::string)convar_giap_map_input_directory)) {
                Console::LogWarning("GIAP : Map Input Directory '%s' does not exist !", ((std::string)convar_giap_map_input_directory).c_str());
            }

            for (auto file : std::filesystem::directory_iterator((std::string)convar_giap_map_input_directory)) {
                if (file.path().extension() == std::filesystem::path(".map")) {
                    CommandToken_t("giap_compile ");
                    GiapCompileImpl(file.path().stem().string().c_str());
                }
            }
        }

        // ------------------------------------------------------------------------------------------------------

        void GiapCompileImpl(const char *map_name) {

            std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();

            StringBuffer entities_key_values_string_buffer{1024};

            if(!std::filesystem::exists((std::string)convar_giap_map_input_directory)) {
                Console::LogWarning("GIAP : Map Input Directory '%s' does not exist !", ((std::string)convar_giap_map_input_directory).c_str());
                return;
            }

            const size_t map_name_size = strlen(map_name);
            const size_t map_input_path_size = ((std::string)convar_giap_map_input_directory).size();
            char *map_path = new char[map_input_path_size + map_name_size + 5];
            sprintf_s(map_path, map_input_path_size + map_name_size + 5, "%s%s.map", ((std::string)convar_giap_map_input_directory).c_str(), map_name);
            map_path[map_input_path_size + map_name_size + 4] = '\0';

            std::ifstream trenchbroom_map_input{};
            trenchbroom_map_input.open(map_path);
            if(trenchbroom_map_input) {

                std::vector<Brush_t> brushes{};
                std::vector<Entity_t> entities{};

                ParseTrenchbroomMap(trenchbroom_map_input, brushes, entities, entities_key_values_string_buffer);

                std::vector<DottedBrush_t> dotteds{};

                BrushesToDottedBrushes(brushes, dotteds);

                std::vector<Mesh_t> meshes{};

                DottedBrushesToMeshes(dotteds, meshes);

                //OUTPUT
                for(const char *output_dir : GIAP_MAP_OUTPUT_DIRECTORIES) {

                    // Generate directories
                    std::filesystem::path dir{output_dir};
                    dir.append(map_name);
                    if (!std::filesystem::is_directory(dir)) {
                        std::filesystem::create_directories(dir);
                    }
                    std::filesystem::path col_dir = dir;
                    col_dir.append("collisions/");
                    if (!std::filesystem::is_directory(col_dir)){
                        std::filesystem::create_directories(col_dir);
                    }
                    std::filesystem::path vis_dir = dir;
                    vis_dir.append("visuals/");
                    if (!std::filesystem::is_directory(col_dir)){
                        std::filesystem::create_directories(vis_dir);
                    }
    
                    {
    
                        char name[9];
                        for(size_t i = 0; i < meshes.size(); i++) {
                            sprintf_s(name, 9, "%04d.obj", i);
                            name[8] = '\0';
        
                            std::filesystem::path p = col_dir;
                            p.append(name);
        
                            std::ofstream ofs{};
                            ofs.open(p);
                            if(ofs) {
                                WriteMeshToObj(meshes[i], ofs);
                                ofs.close();
                            } else {
                                Console::LogInfo("GIAP : Failed to open collision model output file number %d", i);
                            }
                        }
                    }
    
                    {
                        char *name = new char[map_name_size + 6];
                        sprintf_s(name, map_name_size + 6, "%s.gmap", map_name);
                        name[map_name_size + 5] = '\0';
    
                        std::filesystem::path gmap_path = dir;
    
                        gmap_path.append(name);
        
                        std::ofstream ofs{};
                        ofs.open(gmap_path);
                        if(ofs){
                            WriteGMAPFile(map_name, entities, ofs);
                            ofs.close();
                        } else {
                            Console::LogInfo("GIAP : Faialed to open gmap output file '%s'!", gmap_path);
                        }
        
                        std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
                        Console::LogInfo("Done generating map '%s' in %.2f seconds.", map_name, (end_time - start_time).count());
    
                        delete[] name;
                    }
                }

            } else {
                Console::LogInfo("GIAP : Could not open %s", map_path);
            }

            delete[] map_path;
        }

        void ParseTrenchbroomMap(std::ifstream &file, std::vector<Brush_t> &outBrushes, std::vector<Entity_t> &outEntities, StringBuffer &keyValuesStringBuffer) {

            char line_buff[255];
            size_t indentation_depth = 0;

            int entity_count = 0; //0th entity is the world (brushes)

            Brush_t *current_brush{};
            Entity_t *current_entity{};

            while(!file.eof()) {
                file.getline(line_buff, 255);

                if(line_buff[0] == '/' && line_buff[1] == '/') {
                    continue;
                } else if(line_buff[0] == '{') {
                    indentation_depth++;
                    

                    if(indentation_depth == 1) {
                        entity_count++;
                        if(entity_count != 1) {
                            outEntities.emplace_back(Entity_t{});
                            current_entity = &outEntities[outEntities.size() - 1];
                        }
                    }
                    if(indentation_depth == 2) {
                        if(entity_count == 1) {
                            //new brush
                            outBrushes.emplace_back(Brush_t{});
                            current_brush = &outBrushes[outBrushes.size() - 1];
                        }
                    }
                } else if(line_buff[0] == '}') {
                    indentation_depth--;
                } 
                else if(line_buff[0] == '(') {
                    if(indentation_depth == 2) {
                        //new brush line

                        char *pch = strtok(line_buff, " ");

                        bool in_point = false;
                        int point_index = 0;
                        int coord_index = 0;

                        glm::vec3 points[3]{};

                        while (pch) {
                            if (pch[0] == '(') {
                                in_point = true;
                            } else if (pch[0] == ')') {
                                in_point = false;
                                point_index++;
                            } else {
                                if (in_point) {
                                    points[point_index][coord_index] = atof(pch);
                                    coord_index++;
                                    if(coord_index > 2) {
                                        coord_index = 0;
                                    }
                                }
                            }
                            pch = strtok(nullptr, " ");
                        }

                        Plane_t plane{};
                        plane.Normal = glm::normalize(glm::cross(points[1] - points[0], points[2] - points[0]));
                        plane.C = glm::dot(points[1], plane.Normal);

                        current_brush->Planes.emplace_back(plane);
                    }
                } else if(line_buff[0] == '\"') {
                    if(entity_count > 1) {
                        //Parese entity key-value
                        //spectial case for 'origin' which becomes 'Position' and 'angle' which becomes 'StartRotation' (y component)
    
                        char *pch = strtok(line_buff, " ");
                        bool is_origin_key = false;
    
                        if(strcmp(pch, "\"classname\"") == 0) {
                            pch = strtok(nullptr, " ");
                            
                            current_entity->Name = keyValuesStringBuffer.PushWord(pch);
                        } 
                        else if(strcmp(pch, "\"origin\"") == 0) {
                            current_entity->Keys.emplace_back("\"Position\"");
                            is_origin_key = true;
                            goto values;
                        }
                        else if(strcmp(pch, "\"angle\"") == 0) {
                            current_entity->Keys.emplace_back("\"StartRotation\"");
                            pch = strtok(nullptr, " ");
                            current_entity->KeysValueCount.emplace_back(3);
                            current_entity->Values.emplace_back("\"0\"");
                            current_entity->Values.emplace_back(keyValuesStringBuffer.PushWord(pch));
                            current_entity->Values.emplace_back("\"0\"");
                        } 
                        else {
                            current_entity->Keys.emplace_back(keyValuesStringBuffer.PushWord(pch));

                            values:

                            // VALUES
                            {
                                int value_count = 0;
                                pch = strtok(nullptr, " ");
                                while(pch) {
                                    value_count++;

                                    int i = 0;
                                    keyValuesStringBuffer.PushChar('\"');
                                    while(pch[i] != '\0') {

                                        if(pch[i] == '\\') {
                                            i++;
                                            keyValuesStringBuffer.PushChar(pch[i]);
                                        }
                                        if(pch[i] != '\"') {
                                            keyValuesStringBuffer.PushChar(pch[i]);
                                        }
                                        
                                        i++;
                                    }
                                    keyValuesStringBuffer.PushChar('\"');

                                    current_entity->Values.emplace_back(keyValuesStringBuffer.EndWord());

                                    pch = strtok(nullptr, " ");
                                }
                                current_entity->KeysValueCount.emplace_back(value_count);
                            }

                            if(is_origin_key) {
                                //for position, we need to scale it and correct (z-up to y-up).
                                char number[32];
                                glm::vec3 corrected_pos{};
                                for(int i = 0; i < 3; i++) {
                                    const char *original = current_entity->Values[current_entity->Values.size() - i - 1];
                                    float n = atof(original+1);
                                    n = n * (float)convar_giap_scale;
                                    corrected_pos[2-i] = n;
                                }
                                float temp = corrected_pos.y;
                                corrected_pos.y = corrected_pos.z;
                                corrected_pos.z = temp;
                                corrected_pos.x = corrected_pos.x;

                                temp = corrected_pos.x;
                                corrected_pos.x = -corrected_pos.z;
                                corrected_pos.z = temp;

                                for(int i = 0; i < 3; i++) {
                                    sprintf_s(number, 32, "\"%f\"", corrected_pos[i]);
                                    current_entity->Values[current_entity->Values.size() - i - 1] = keyValuesStringBuffer.PushWord(number);
                                }
                            }
                        }
                    }
                    
                } else {
                    //dont care
                }
            }
        }

        void BrushesToDottedBrushes(const std::vector<Brush_t> &brushes, std::vector<DottedBrush_t> &outDotteds) {

            DottedBrush_t *current_dotted{};

            for (const Brush_t &brush : brushes) {

                if (brush.Planes.size() < 3) {
                    Console::LogWarning("GIAP : Invalid brush : less than 3 planes.\n");
                    continue;
                }

                outDotteds.emplace_back();
                current_dotted = &outDotteds[outDotteds.size() - 1];
                current_dotted->Original = &brush;
                current_dotted->PerPlaneVertices.resize(brush.Planes.size());

                for (int i = 0; i < brush.Planes.size() - 2; i++) {

                    for (int j = i + 1; j < brush.Planes.size() - 1; j++) {

                        for (int k = j + 1; k < brush.Planes.size(); k++) {

                            const Plane_t &A = brush.Planes[i];
                            const Plane_t &B = brush.Planes[j];
                            const Plane_t &C = brush.Planes[k];

                            
                            // Find intersection of the 3 planes using cramer's rule.

                            float t = glm::dot(A.Normal, glm::cross(B.Normal, C.Normal));

                            if ((t < 0.0f ? -t : t) <= 0.000001f) {
                                continue;
                            }


                            glm::vec3 P{};

                            glm::vec3 L1 = A.Normal;
                            glm::vec3 L2 = B.Normal;
                            glm::vec3 L3 = C.Normal;
                            L1.x = A.C;
                            L2.x = B.C;
                            L3.x = C.C;
                            P.x = glm::dot(L1, glm::cross(L2, L3)) / t;

                            L1.x = A.Normal.x;
                            L2.x = B.Normal.x;
                            L3.x = C.Normal.x;
                            L1.y = A.C;
                            L2.y = B.C;
                            L3.y = C.C;
                            P.y = glm::dot(L1, glm::cross(L2, L3)) / t;

                            L1.y = A.Normal.y;
                            L2.y = B.Normal.y;
                            L3.y = C.Normal.y;
                            L1.z = A.C;
                            L2.z = B.C;
                            L3.z = C.C;
                            P.z = glm::dot(L1, glm::cross(L2, L3)) / t;

                            //Check if the point is actually within the brushe's bounds
                            bool valid = true;
                            for (int m = 0; m < brush.Planes.size(); m++) {
                                if (m != i && m != j && m != k) {
                                    if (glm::dot(P, brush.Planes[m].Normal) < brush.Planes[m].C - 0.00001f) {
                                        valid = false;
                                        break;
                                    }
                                }
                            }

                            if (valid) {
                                current_dotted->Vertices.emplace_back(P);
                                int new_vert_index = current_dotted->Vertices.size() - 1;
                                current_dotted->PerPlaneVertices[i].emplace_back(new_vert_index);
                                current_dotted->PerPlaneVertices[j].emplace_back(new_vert_index);
                                current_dotted->PerPlaneVertices[k].emplace_back(new_vert_index);
                            }
                        }
                    }
                }
            }
        }

        void DottedBrushesToMeshes(const std::vector<DottedBrush_t> &dotteds, std::vector<Mesh_t> &outMeshes) {

            Mesh_t *current_mesh{};

            outMeshes.reserve(dotteds.size());

            for(size_t i = 0; i < dotteds.size(); i++) {

                outMeshes.emplace_back();
                current_mesh = &outMeshes[outMeshes.size() - 1];

                current_mesh->VerticesCount = dotteds[i].Vertices.size();
                current_mesh->Vertices = &dotteds[i].Vertices[0];

                size_t plane_count = dotteds[i].PerPlaneVertices.size();

                current_mesh->Normals.reserve(plane_count);
                for(size_t j = 0; j < plane_count; j++) {

                    const Plane_t &plane = dotteds[i].Original->Planes[j];
                    std::vector<int> plane_indices = dotteds[i].PerPlaneVertices[j]; //copy cause we WILL mutate it. :(

                    current_mesh->Normals.emplace_back(plane.Normal);

                    std::vector<int> ordered_indices{};
                    ordered_indices.reserve(plane_indices.size());

                    glm::vec3 centroid{};
                    for(int index : plane_indices) {
                        centroid += dotteds[i].Vertices[index];
                    }
                    centroid = centroid / (float)plane_indices.size();

                    //select two perpendicular basis vector within the plane
                    glm::vec3 basis_x{}, basis_y{};
                    {
                        glm::vec3 any{0.7071, 0.7071, 0.0f}; // can be any unit vector not colinear to normal
                        basis_x = glm::cross(any, plane.Normal);
                        if (basis_x.x == 0.0f && basis_x.y == 0.0f && basis_x.z == 0.0f) {
                            // unlucky, it was colinear, choose another one
                            any.x = 1;
                            any.y = 0;
                            basis_x = glm::cross(any, plane.Normal);
                        }
                        basis_y = glm::cross(basis_x, plane.Normal);
                    }

                    // find the next index going in radial order
                    // then remove it from the list and get a new one
                    // essenntialy sorting ordered_indices in radial order.
                    for(int j = 0; j < plane_indices.size(); j++) {

                        int best_index = -1;
                        int best_index_pos = -1;
                        float best_x{};
                        float best_y{};

                        for (int k = 0; k < plane_indices.size(); k++) {

                            int index = plane_indices[k];

                            if (index == -1) {
                                //this one was already used
                                continue;
                            }

                            glm::vec3 vert = dotteds[i].Vertices[index];

                            glm::vec3 dir = normalize(vert - centroid);
                            float on_x = glm::dot(dir, basis_x);
                            float on_y = glm::dot(dir, basis_y);

                            if (best_index == -1) {
                                //this is the first
                                best_index = index;
                                best_x = on_x;
                                best_y = on_y;
                                best_index_pos = k;
                            } else {
                                if (best_y > 0.0f) {
                                    if (on_y < 0.0f) {
                                        continue;
                                    } else {
                                        if (on_x > best_x) {
                                            best_index = index;
                                            best_x = on_x;
                                            best_y = on_y;
                                            best_index_pos = k;
                                            continue;
                                        }
                                    }
                                } else if (best_y < 0.0f){
                                    if (on_y > 0.0f) {
                                        best_index = index;
                                        best_x = on_x;
                                        best_y = on_y;
                                        best_index_pos = k;
                                        continue;
                                    } else {
                                        if (on_x < best_x) {
                                            best_index = index;
                                            best_x = on_x;
                                            best_y = on_y;
                                            best_index_pos = k;
                                            continue;
                                        }
                                    }
                                } else {
                                    if (on_y > 0.0f || (on_y == 0.0f && on_x > 0.0f)) {
                                        best_index = index;
                                        best_x = on_x;
                                        best_y = on_y;
                                        best_index_pos = k;
                                        continue;
                                    }
                                }
                            }
                        }

                        ordered_indices.emplace_back(best_index);
                        if(best_index_pos >= plane_indices.size()) {
                            Console::LogError("GIAP : HUGE ERROR");
                            return;
                        }
                        plane_indices[best_index_pos] = -1; // -1 means that we wont get it next loop. faster than using remove();
                    }
                    // plane_indices is now invalid, btw
                    // ordered_indices contains every indices in radial order.

                    if(ordered_indices.size() < 3) {
                        Console::LogInfo("GIAP : Invalid Mesh Face : less than 3 vertices");
                        return;
                    }

                    //We now triangulate the face (plane) and add it to the mesh.
                    Mesh_t::ObjVert_t O{};
                    O.PosIndex = ordered_indices[0];
                    O.NormalIndex = j;
                    for(int k = 0; k < ordered_indices.size() - 1; k++) {
                        current_mesh->Indices.emplace_back(O);

                        Mesh_t::ObjVert_t V{};
                        V.PosIndex = ordered_indices[k];
                        V.NormalIndex = j;
                        current_mesh->Indices.emplace_back(V);

                        V.PosIndex = ordered_indices[ k + 1];
                        V.NormalIndex = j;
                        current_mesh->Indices.emplace_back(V);
                    }
                }
            }
        }

        void WriteMeshToObj(const Mesh_t &mesh, std::ofstream &output) {

            output << "# collision mesh generated with Gigno Map Generator\n";

            for(int i = 0; i < mesh.VerticesCount; i++) {
                glm::vec3 vert = mesh.Vertices[i] * (float)convar_giap_scale;
                output << "v " << vert.x << " " << vert.z << " " << -vert.y << "\n";
            }

            for(glm::vec3 normal : mesh.Normals) {
                output << "vn " << -normal.x << " " << -normal.y << " " << -normal.z << "\n";
            }

            output << "s off\n";

            for(int i = 0; i < mesh.Indices.size(); i++) {
                output << "f ";
                output << mesh.Indices[i].PosIndex + 1 << "//" << mesh.Indices[i].NormalIndex + 1 << " ";
                i++;
                output << mesh.Indices[i].PosIndex + 1 << "//" << mesh.Indices[i].NormalIndex + 1 << " ";
                i++;
                output << mesh.Indices[i].PosIndex + 1 << "//" << mesh.Indices[i].NormalIndex + 1 << " ";
                output << "\n";
            }

        }

        void WriteGMAPFile(const char *mapName, const std::vector<Entity_t> &entities, std::ofstream &output){
            // Write WorldSpawn entity
            output << "\"WorldSpawn\"  {\n"
                            "\t\"VisualsScale\" : \""                   << (float)convar_giap_scale << "\" ,\n"
                        "}\n";

            output << "\n";

            for(const Entity_t &entity : entities) {
                output << entity.Name << "  {\n";
                
                int v = 0;
                for(int i = 0; i < entity.Keys.size(); i++) {
                    output << "\t" << entity.Keys[i] << " : ";
                    for(int j = 0; j < entity.KeysValueCount[i]; j++) {
                        output << entity.Values[v] << " ";
                        v++;
                    }
                    output << ",\n";
                }

                output << "}\n\n";
            }

            output << "\n";
        }
    }
}

#endif

