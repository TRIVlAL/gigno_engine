cmake_minimum_required(VERSION 3.25)
project(Gigno LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VULKAN_SDK_PATH C:/dev_libraries/VulkanSDK)
set(GLFW_PATH C:/dev_libraries/glfw/glfw-3.4.bin.WIN64)
set(GLFW_LIBRARY_FOLDER lib-mingw-w64)

#Copy shaders
file(GLOB_RECURSE SPV_FILES "${PROJECT_SOURCE_DIR}/shaders/*.spv")

foreach(SHADER ${SPV_FILES})
    cmake_path(GET SHADER FILENAME SHADER_FILE_NAME)
    configure_file(${SHADER} ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_FILE_NAME} COPYONLY)
endforeach(SHADER ${SHADER_FILES})

#Copy models
file(GLOB_RECURSE OBJ_FILES "${PROJECT_SOURCE_DIR}/models/*.obj")
foreach(OBJ ${OBJ_FILES})
    cmake_path(GET OBJ FILENAME OBJ_NAME)
    configure_file(${OBJ} ${CMAKE_CURRENT_BINARY_DIR}/models/${OBJ_NAME} COPYONLY)
endforeach(OBJ ${OBJ_FILES})

#Copy maps
file(GLOB_RECURSE MAP_FILES "${PROJECT_SOURCE_DIR}/maps/*.map")
foreach(MAP ${MAP_FILES})
    cmake_path(GET MAP FILENAME MAP_NAME)
    configure_file(${MAP} ${CMAKE_CURRENT_BINARY_DIR}/maps/${MAP_NAME} COPYONLY)
endforeach(MAP ${MAP_FILES})


set (EXCLUDE_DIR "/vendor/glm/")
file (GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
foreach (TMP_PATH ${SOURCES})
    string (FIND ${TMP_PATH} ${EXCLUDE_DIR} EXCLUDE_DIR_FOUND)
    if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
        list (REMOVE_ITEM SOURCES ${TMP_PATH})
    endif ()
endforeach(TMP_PATH)

add_link_options(--static -static-libgcc -static-libstdc++) # Required for program to work outside of IDE.

add_executable(gigno ${SOURCES})

target_include_directories(gigno PUBLIC
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src 
    ${PROJECT_SOURCE_DIR}/src/vendor/glm
    ${GLFW_PATH}/include
    ${VULKAN_SDK_PATH}/Include
    ${PROJECT_SOURCE_DIR}/src/vendor/imgui)
target_link_directories(gigno PUBLIC 
    ${VULKAN_SDK_PATH}/Lib
    ${GLFW_PATH}/${GLFW_LIBRARY_FOLDER})
target_link_libraries(gigno glfw3 vulkan-1)
