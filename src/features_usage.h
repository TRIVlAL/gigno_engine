/*
    FEATURES_USAGE.H

Macros for enabling/disableling various core features of the engine. 
Some features depends/require on other features. Those precede the '1' or '0' value with a chain of '&&'

Some features (namely Console, Profiler, m.b. other) still define theirpulbic method. In these cases, the methods are most likely empty.
*/

//LOGGING PREFE RENCES
#define VERBOSE 0
#define LOG_VULKAN_INFOS 0

//IMGUI
#define USE_IMGUI 1

//DEBUGGING                                    DEPENDENCIES                  VALUES
#define USE_DEBUG_DRAWING                                                       1

#define USE_DEBUG_SERVER                                                        1
#define USE_CONSOLE                 (USE_DEBUG_SERVER && USE_IMGUI &&           1 )
#define USE_PROFILER                (USE_DEBUG_SERVER && USE_IMGUI &&           1 )
