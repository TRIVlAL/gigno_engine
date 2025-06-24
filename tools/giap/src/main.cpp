
#include <vector>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cmath>
#include <filesystem>

struct Point_t {
    float x{}, y{}, z{};
};

Point_t add(Point_t a, Point_t b) { Point_t ret{}; ret.x = a.x + b.x; ret.y = a.y + b.y; ret.z = a.z + b.z; return ret;}
Point_t minus(Point_t p) {p.x = -p.x; p.y = -p.y; p.z = -p.z; return p;}
Point_t cross(Point_t a, Point_t b) {Point_t ret{}; ret.x = a.y*b.z - a.z*b.y; ret.y = a.z*b.x - a.x*b.z; ret.z = a.x*b.y - a.y*b.x; return ret;}
Point_t scal_mult(float s, Point_t p) {p.x = p.x*s; 
    p.y = p.y*s; 
    p.z = p.z*s; 
    return p;}
float mag2(Point_t p) {return p.x*p.x + p.y*p.y + p.z*p.z;}
float mag(Point_t p) {return sqrt(mag2(p));}
Point_t normalize(Point_t p) {return scal_mult(1/mag(p), p); }
float dot(Point_t a, Point_t b) {return a.x*b.x + a.y*b.y + a.z*b.z;}

float fabs(float x) {return x < 0.0f ? -x : x;}

struct Plane_t {
    Point_t normal;
    float c;
};

struct Arguments_t {
    char* filepath{};
    bool nocol{};
    bool verbose{};
    float scale{0.1f};
} s_Arguments{};

const char MAP_DIRECTORY_PATH [] = "../../../assets/maps/";

struct Brush_t {
    std::vector<Plane_t> planes{};
};

// Representation inbetween a Brush and a Mesh :
// All the vertices are in place, as well as on what plane they're on.
// Only thing missing : the triangulation of those vertices.
struct DottedBrush_t {
    Brush_t *original{};

    std::vector<Point_t> vertices{}; //list of every vertices composing the brush

    std::vector<std::vector<int>> per_plane_vertices{}; // for every plane in the original brush, a vector of indices within the vertices array
};

struct Mesh_t {
    int vertices_count{};
    Point_t *vertices{};

    std::vector<Point_t> normals{};

    struct Vert_t {
        int pos_indx{};
        int normal_indx{};
    };
    std::vector<Vert_t> indices{};
};

#define printf_v(...) if(s_Arguments.verbose) {printf(__VA_ARGS__);}

void parse_brush_line(Brush_t *output, char *input);
int filepath_to_filename(char *filepath, char *out_filename);

/*
Given a NAME.map file generated with Trenchbroom, this program will create a folder containing :

- One .obj file called 'map_NAME.obj'
- A folder containing many .obj file called : 'map_collision_NAME_0001' to 'map_collision_NAME_xxxx'

The following assumptions are made about the .map file :
- each brush face ttakes one line
- each brace takes one line
- worldspawn is the first content of the file.


EXAMPLE OF INPUT :
---------------------------------------------------------------------
// Game: Generic
// Format: Standard
// entity 0
{
"classname" "worldspawn"
// brush 0
{
( 128 64 192 ) ( 128 0 192 ) ( 128 0 16 ) __TB_empty 0 0 0 1 1
( 128 0 192 ) ( 192 0 192 ) ( 192 0 16 ) __TB_empty 0 0 0 1 1
( 192 0 16 ) ( 192 64 16 ) ( 128 64 16 ) __TB_empty 0 0 0 1 1
( 128 64 192 ) ( 192 64 192 ) ( 192 0 192 ) __TB_empty 0 0 0 1 1
( 192 64 16 ) ( 192 64 192 ) ( 128 64 192 ) __TB_empty 0 0 0 1 1
( 192 0 192 ) ( 192 64 192 ) ( 192 64 16 ) __TB_empty 0 0 0 1 1
}
// brush 1
{
( 320 192 128 ) ( 320 128 128 ) ( 320 128 16 ) __TB_empty 0 0 0 1 1
( 320 128 128 ) ( 384 128 128 ) ( 384 128 16 ) __TB_empty 0 0 0 1 1
( 384 128 16 ) ( 384 192 16 ) ( 320 192 16 ) __TB_empty 0 0 0 1 1
( 320 192 128 ) ( 384 192 128 ) ( 384 128 128 ) __TB_empty 0 0 0 1 1
( 352 160 128 ) ( 352 192 96 ) ( 480 192 96 ) __TB_empty 0 0 0 1 1
( 384 192 16 ) ( 384 192 128 ) ( 320 192 128 ) __TB_empty 0 0 0 1 1
( 384 128 128 ) ( 384 192 128 ) ( 384 192 16 ) __TB_empty 0 0 0 1 1
}
// brush 2
{
( 0 192 192 ) ( 0 128 192 ) ( 0 128 16 ) __TB_empty 0 0 0 1 1
( 352 144 128 ) ( 352 128 16 ) ( 32 144 128 ) __TB_empty 0 0 0 1 1
( 64 128 16 ) ( 64 192 16 ) ( 0 192 16 ) __TB_empty 0 0 0 1 1
( 0 192 192 ) ( 64 192 192 ) ( 64 128 192 ) __TB_empty 0 0 0 1 1
( 64 192 16 ) ( 64 192 192 ) ( 0 192 192 ) __TB_empty 0 0 0 1 1
( 64 128 192 ) ( 64 192 192 ) ( 64 192 16 ) __TB_empty 0 0 0 1 1
}
// brush 3
{
( 128 192 96 ) ( 128 128 96 ) ( 128 128 16 ) __TB_empty 0 0 0 1 1
( 128 128 96 ) ( 256 128 96 ) ( 256 128 16 ) __TB_empty 0 0 0 1 1
( 256 128 16 ) ( 256 192 16 ) ( 128 192 16 ) __TB_empty 0 0 0 1 1
( 128 192 96 ) ( 256 192 96 ) ( 256 128 96 ) __TB_empty 0 0 0 1 1
( 192 160 96 ) ( 192 192 64 ) ( 320 192 64 ) __TB_empty 0 0 0 1 1
( 256 192 16 ) ( 256 192 96 ) ( 128 192 96 ) __TB_empty 0 0 0 1 1
( 256 128 96 ) ( 256 192 96 ) ( 256 192 16 ) __TB_empty 0 0 0 1 1
}
// brush 4
{
( -192 192 16 ) ( -192 -64 16 ) ( -192 -64 -16 ) __TB_empty 0 0 0 1 1
( -128 -64 0 ) ( -192 0 0 ) ( -192 0 128 ) __TB_empty 0 0 0 1 1
( -192 128 0 ) ( -128 192 0 ) ( -128 192 128 ) __TB_empty 0 0 0 1 1
( -192 -64 16 ) ( 256 -64 16 ) ( 256 -64 -16 ) __TB_empty 0 0 0 1 1
( 256 -64 -16 ) ( 256 192 -16 ) ( -192 192 -16 ) __TB_empty 0 0 0 1 1
( -192 192 16 ) ( 256 192 16 ) ( 256 -64 16 ) __TB_empty 0 0 0 1 1
( 256 192 -16 ) ( 256 192 16 ) ( -192 192 16 ) __TB_empty 0 0 0 1 1
( 192 -64 0 ) ( 256 0 128 ) ( 256 0 0 ) __TB_empty 0 0 0 1 1
( 256 -64 16 ) ( 256 192 16 ) ( 256 192 -16 ) __TB_empty 0 0 0 1 1
}
}
---------------------------------------------------------------------
*/
int main(int argC, char *argV[]) {

    if(argC == 1) {
        printf("map_gen [map_file_path]\nOptions : -nocol -verbose\n");
        return 1;
    }
    
    if(argV[1][0] == '-') {
        printf("map_gen [map_file_path]\nOptions : -nocol -verbose -scale 0.1\nmap_file_path MUST BE SET\n");
        return 1;
    } else {
        s_Arguments.filepath = argV[1];
    }

    for(int i = 2; i < argC; i++) {
        if(argV[i][0] != '-') {
            break;
        }
        if(strcmp(argV[i], "-nocol") == 0) {
            s_Arguments.nocol = true;
        }
        if(strcmp(argV[i], "-verbose") == 0) {
            s_Arguments.verbose = true;
        }
        if(strcmp(argV[i], "-scale") == 0) {
            if(i + 1 < argC) {
                s_Arguments.scale = atof(argV[i+1]);
            }
        }
    }

    printf_v("Verbose ON\n");

    char line_buff[255];
    int depth = 0;

    std::vector<Brush_t> brushes{};
    Brush_t *current_brush{};

    std::ifstream map{s_Arguments.filepath};
    if(!map) {
        printf("Could not open file %s\n", s_Arguments.filepath);
        return 2;
    }

    printf_v("Begining Parsing .map file :\n\n");

    while(map.peek() != EOF) {
        map.getline(line_buff, 255);

        if(line_buff[0] == '/' && line_buff[1] == '/') {
            printf_v("COMMENT\n");
            continue;
        } else  if(line_buff[0] == '{') {
            printf_v("OPEN BRACE\n");
            depth++;
            if(depth == 2) {
                brushes.emplace_back();
                current_brush = &brushes[brushes.size() - 1];
            }
        } else if(line_buff[0] == '}') {
            printf_v("CLOSE BRACE\n");
            depth--;
            if(depth == 0) {
                printf_v("DONE\n\n\n");
                break;
            }
        } else if(line_buff[0] == '(' && current_brush) {
            printf_v("BRUSH FACE\n");
            parse_brush_line(current_brush, line_buff);
        } else {
            printf_v("OTHER\n");
            continue;
        }
    }

    for(Brush_t brush : brushes) {
        printf_v("\n\nBrush : \n")
        int i = 0;
        for(Plane_t plane : brush.planes) {
            printf_v("Plane %d : (%f %f %f) -> %f\n",i++, plane.normal.x, plane.normal.y, plane.normal.z, plane.c);
        }
    }

    printf_v("Creating Dottening the Brushes\n\n");

    std::vector<DottedBrush_t> dotteds{};
    dotteds.reserve(brushes.size());
    DottedBrush_t *current_dotted{}; 

    int brush_indx = 0;

    for(Brush_t& brush : brushes) {

        if(brush.planes.size() < 3) {
            printf_v("Invalid brush : less than 3 planes.\n");
            continue;
        }

        dotteds.emplace_back();
        current_dotted = &dotteds[dotteds.size() - 1];
        current_dotted->original = &brush;
        current_dotted->per_plane_vertices.resize(brush.planes.size());

        for(int i = 0; i < brush.planes.size() - 2; i++) {
            for(int j = i+1; j < brush.planes.size() - 1; j++) {
                for(int k = j+1; k < brush.planes.size(); k++) {

                    Plane_t &A = brush.planes[i];
                    Plane_t &B = brush.planes[j];
                    Plane_t &C = brush.planes[k];

                    float t = dot(A.normal, cross(B.normal, C.normal));

                    if((t < 0.0f ? -t : t) <= 0.000001f) {
                        continue;
                    }

                    //Find intersection using cramer's rule
                    
                    Point_t P{};

                    Point_t L1 = A.normal;
                    Point_t L2 = B.normal;
                    Point_t L3 = C.normal;
                    L1.x = A.c;
                    L2.x = B.c;
                    L3.x = C.c;
                    P.x = dot(L1, cross(L2, L3))/t;

                    L1.x = A.normal.x;
                    L2.x = B.normal.x;
                    L3.x = C.normal.x;
                    L1.y = A.c;
                    L2.y = B.c;
                    L3.y = C.c;
                    P.y = dot(L1, cross(L2, L3))/t;

                    L1.y = A.normal.y;
                    L2.y = B.normal.y;
                    L3.y = C.normal.y;
                    L1.z = A.c;
                    L2.z = B.c;
                    L3.z = C.c;
                    P.z = dot(L1, cross(L2, L3))/t;

                    bool valid = true;
                    for(int m = 0; m < brush.planes.size(); m++) {
                        if(m != i && m != j && m != k) {
                            if(dot(P, brush.planes[m].normal) < brush.planes[m].c - 0.00001f ) {
                                valid = false;
                                printf_v("Dissmissed : P(%f, %f, %f)", P.x, P.y, P.z);
                                printf_v(" generated from planes at indices %d, %d, and %d\n", i, j, k);
                                break;
                            }
                        }
                    }

                    if(valid) {
                        current_dotted->vertices.emplace_back(P);
                        int new_vert_index = current_dotted->vertices.size() - 1;
                        current_dotted->per_plane_vertices[i].emplace_back(new_vert_index);
                        current_dotted->per_plane_vertices[j].emplace_back(new_vert_index);
                        current_dotted->per_plane_vertices[k].emplace_back(new_vert_index);
                    }
                }
            }
        }
    }

    if(s_Arguments.verbose) {

        for(int i = 0; i < dotteds.size(); i++) {
            printf_v("\nDotted mesh %d :\n", i)
            printf_v("Vertices:\n");
            for(int j = 0; j < dotteds[i].vertices.size(); j++) {
                printf_v("     (%f, %f, %f)\n", dotteds[i].vertices[j].x, dotteds[i].vertices[j].y, dotteds[i].vertices[j].z);
            }
            printf_v("Per plane indices:\n");
            for(int j = 0; j < dotteds[i].per_plane_vertices.size(); j++) {
                printf_v("     Plane %02d : (", j);
                for(int k = 0; k < dotteds[i].per_plane_vertices[j].size(); k++) {
                    printf_v("%d,", dotteds[i].per_plane_vertices[j][k]);
                }
                Point_t n = dotteds[i].original->planes[j].normal;
                printf_v(")              (from (%f, %f, %f) -> %f)\n", n.x, n.y, n.z, dotteds[i].original->planes[j].c);
            }
            printf_v("\n");
        }
    }


    printf_v("\n\nTriangulating Meshes\n\n");

    std::vector<Mesh_t> meshes{};
    Mesh_t *current_mesh{};
    meshes.reserve(dotteds.size());
    for(int i = 0; i < dotteds.size(); i++) {
        meshes.emplace_back();
        current_mesh = &meshes[meshes.size() - 1];

        current_mesh->vertices_count = dotteds[i].vertices.size();
        current_mesh->vertices = &dotteds[i].vertices[0];
        current_mesh->normals.reserve(dotteds[i].per_plane_vertices.size());

        for(int j = 0; j < dotteds[i].per_plane_vertices.size(); j++) {

            Point_t normal = dotteds[i].original->planes[j].normal;
            current_mesh->normals.emplace_back(normal);

            std::vector<int> ordered_indices{};
            ordered_indices.resize(dotteds[i].per_plane_vertices[j].size());

            Point_t centroid{};
            for(int k = 0; k < dotteds[i].per_plane_vertices[j].size(); k++) {
                centroid = add(centroid, dotteds[i].vertices[dotteds[i].per_plane_vertices[j][k]]);
            }
            centroid = scal_mult(1/(float)(dotteds[i].per_plane_vertices[j].size()), centroid);

            //Yes this is a very dumb sorting; i can't be bothered to cook up anything better.
            //Radial direction sorting, from angle = 0 to angle = 2pi
            for(int k = 0; k < dotteds[i].per_plane_vertices[j].size(); k++) {

                int best_index = -1;
                int best_index_pos = -1;
                float best_x{};
                float best_y{};

                Point_t any{};
                any.y = 1;
                Point_t basis_x = cross(any, normal);
                if(basis_x.x == 0.0f && basis_x.y == 0.0f && basis_x.z == 0.0f) {
                    any.x = 1;
                    any.y = 0;
                    basis_x = cross(any, normal);
                }
                Point_t basis_y = cross(basis_x, normal);
                
                for(int l = 0; l < dotteds[i].per_plane_vertices[j].size(); l++) {
                    int index = dotteds[i].per_plane_vertices[j][l];
                    if(index == -1) {
                        continue;
                    }
                    Point_t vert = dotteds[i].vertices[index];

                    Point_t dir = normalize(add(vert, minus(centroid)));
                    float on_x = dot(dir, basis_x);
                    float on_y = dot(dir, basis_y);
                    
                    if(best_index == -1) {
                        best_index = index;
                        best_x = on_x;
                        best_y = on_y;
                        best_index_pos = l;
                    } else {
                        if(best_y > 0.0f) {
                            if(on_y < 0.0f) {
                                continue;
                            } else {
                                if(on_x > best_x) {
                                    best_index = index;
                                    best_x = on_x;
                                    best_y = on_y;
                                    best_index_pos = l;
                                     continue;
                                }
                            }
                        } else if(best_y < 0.0f) {
                            if(on_y > 0.0f) {
                                best_index = index;
                                best_x = on_x;
                                best_y = on_y;
                                best_index_pos = l;
                                 continue;
                            } else {
                                if(on_x < best_x) {
                                    best_index = index;
                                    best_x = on_x;
                                    best_y = on_y;
                                    best_index_pos = l;
                                    continue;
                                }
                            }
                        } else {
                            if(on_y > 0.0f || (on_y == 0.0f && on_x > 0.0f)) {
                                best_index = index;
                                best_x = on_x;
                                best_y = on_y;
                                best_index_pos = l;
                                continue;
                            }
                        }
                    }
                }

                ordered_indices[k] = best_index;
                dotteds[i].per_plane_vertices[j][best_index_pos] = -1; //so that we get a new one on next loop. faster than using remove();
            }

            if(ordered_indices.size() < 3) {
                printf_v("Invalid mesh indices : only 2 vertices!!!");
                return 4;
            }
            Mesh_t::Vert_t O{};
            O.pos_indx = ordered_indices[0];
            O.normal_indx = j;
            for(int k = 1; k < ordered_indices.size() - 1; k++) {
                current_mesh->indices.emplace_back(O);

                Mesh_t::Vert_t V{};
                V.pos_indx = ordered_indices[k];
                V.normal_indx = j;
                current_mesh->indices.emplace_back(V);

                V.pos_indx = ordered_indices[k+1];
                V.normal_indx = j;
                current_mesh->indices.emplace_back(V);
            }
        }
    }

    
    
    int map_name_size = filepath_to_filename(s_Arguments.filepath, nullptr) + 1;
    char map_name[map_name_size]{};
    filepath_to_filename(s_Arguments.filepath, map_name);
    map_name[map_name_size - 1] = '\0';
    
    char directory[sizeof(MAP_DIRECTORY_PATH) + map_name_size + 1];
    sprintf_s(directory, sizeof(MAP_DIRECTORY_PATH) + map_name_size + 1, "%s%s/", MAP_DIRECTORY_PATH, map_name);

    printf_v("Generating directories.\n");

    std::filesystem::path dir{(char*)directory};
    if(!std::filesystem::is_directory(dir)) {
        std::filesystem::create_directories(dir);
    }
    std::filesystem::path col_dir = dir;
    col_dir.append("collisions/");
    if(!std::filesystem::is_directory(col_dir)) {
        std::filesystem::create_directories(col_dir);
    }
    std::filesystem::path vis_dir = dir;
    vis_dir.append("visuals/");
    if(!std::filesystem::is_directory(col_dir)) {
        std::filesystem::create_directories(vis_dir);
    }

    
    printf_v("Outputing collisions models\n\n");

    {
        char path[sizeof(directory) + 26];
        //one file per mesh.
        for(int i = 0; i < meshes.size(); i++) {
    
            sprintf_s(path, sizeof(directory) + 18, "%scollisions/%03d.obj", directory, i);
            path[sizeof(directory) + 17] = '\0';
    
            std::ofstream ofs{};
    
            ofs.open(path);
            if(!ofs) {
                printf_v("Failed to open ofstream '%s' : %s", path, std::strerror(errno));
                return 5;
            }
    
            ofs << "#Generated collision data\n";
    
            path[14] = '\0';
            ofs << "o " << path << "\n";
    
            for(int j = 0; j < meshes[i].vertices_count; j++) {
                ofs << "v " << meshes[i].vertices[j].x * s_Arguments.scale << " " << meshes[i].vertices[j].z * s_Arguments.scale << " " << -meshes[i].vertices[j].y * s_Arguments.scale << "\n";
            }
    
            for(int j = 0; j < meshes[i].normals.size(); j++) {
                ofs << "vn " << -meshes[i].normals[j].x << " " << -meshes[i].normals[j].y << " " << -meshes[i].normals[j].z << "\n";
            }
    
            ofs << "s off\n";
    
            for(int j = 0; j < meshes[i].indices.size(); j++) {
                ofs << "f ";
                ofs << meshes[i].indices[j].pos_indx+1 << "//" << meshes[i].indices[j].normal_indx+1 << " ";
                j++;
                ofs << meshes[i].indices[j].pos_indx+1 << "//" << meshes[i].indices[j].normal_indx+1 << " ";
                j++;
                ofs << meshes[i].indices[j].pos_indx+1 << "//" << meshes[i].indices[j].normal_indx+1 << " ";
                ofs << "\n";
            }
    
            ofs.close();
        }
    }


    printf_v("Generating .gmap file\n");
    {
        char path[sizeof(directory) + map_name_size + 4];
        sprintf_s(path, sizeof(directory) + map_name_size + 4, "%s%s.gmap", directory, map_name);
        path[sizeof(directory) + map_name_size + 3] = '\0';

        std::ofstream ofs{};
        ofs.open(path);
        if(!ofs) {
            printf("Failed to open ofstream '%s' : %s", path, std::strerror(errno));
            return 6;
        }

        ofs << "\"WorldSpawn\"  {\n"
                    "\t\"CollisionsPath\" : \"assets/maps/" << map_name << "/collisions/\"\n,"
                    "\t\"VisualsPath\" : \"assets/maps/" << map_name << "/visuals/\"\n,"
                    "\t\"VisualsScale\" : \"" << s_Arguments.scale << "\"\n"
                "}\n"
                "\n"
                "\"DirectionalLight\" {" 
                    "\t\"Intensity\" : \"0.4\",\n"
                    "\t\"Direction\" : \"0.5\" \"-0.7\" \"-0.3\"\n"
                "}\n\n"
                "\"GravgunController\" {\n"
                "}";
    }

    printf("Generated map '%s' in directory '%s'\n", map_name, MAP_DIRECTORY_PATH);

}

void parse_brush_line(Brush_t *output, char *input) {
    char *pch = strtok(input, " ");

    bool in_point = false;
    int point_index = 0;
    int coord_index = 0;

    Point_t points[3]{};

    while(pch) {
        if(pch[0] == '(') {
            in_point = true;
        }
        else if(pch[0] == ')') {
            in_point = false;
            point_index++;
        }
        else {
            if(in_point) {
                if(coord_index == 0) {
                    points[point_index].x = atof(pch);
                    coord_index++;
                } else  if(coord_index == 1) {
                    points[point_index].y = atof(pch);
                    coord_index++;
                } else if(coord_index == 2) {
                    points[point_index].z = atof(pch);
                    coord_index = 0;
                }
            }
        }
        pch = strtok(nullptr, " ");
    }

    printf_v("(%f, %f, %f) (%f, %f, %f) (%f, %f, %f)\n", points[0].x, points[0].y, points[0].z, points[1].x, points[1].y, points[1].z, points[2].x, points[2].y, points[2].z);

    Plane_t plane{};
    Point_t d1 = add(points[1], minus(points[0]));
    Point_t d2 = add(points[2], minus(points[0]));
    plane.normal = normalize(cross(d1, d2));
    plane.c = dot(points[1], plane.normal);
    output->planes.emplace_back(plane);
}

int filepath_to_filename(char *filepath, char *out_filename) {

    /*
    strips a path to it's name. for example: 'full/path/filename.map' -> 'filename'
    */

    int filepath_size = strlen(filepath);
    char *end = filepath + filepath_size;
    int end_count = 0;
    while(*end != '.') {
        end_count++;
        end--;
        if(end_count == filepath_size) {
            //no file extension.
            end_count = 0;
            break;
        }
    }
    char *begin = filepath + filepath_size - end_count;
    int begin_count = end_count;
    while(*begin != '/' && *begin != '\\') {
        begin--;
        if(*begin != '/' && *begin != '\\')
            begin_count++;
        if(begin_count == filepath_size - 1) {
            begin_count = filepath_size;
            break;
        }
    }

    if(out_filename) {
        memcpy(out_filename, (filepath + filepath_size - begin_count), (begin_count - end_count) * sizeof(char));
    }

    return begin_count - end_count;
}
