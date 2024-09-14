#ifndef ERROR_H
#define ERROR_H

#include <iostream>

//prints message and returns
#define ERR_MSG(msg) \
std::cerr << "ERROR at file " __FILE__ " line " << __LINE__ << " : " << msg << std::endl; __debugbreak();\
return\

#define ERR_MSG_V(msg, ret) \
ERR_MSG(msg) ret

#define ERR_V(ret) \
ERR_MSG_V("", ret)

#define ERR \
ERR_MSG("")

//prints message and return value
#define ERR_NULL_MSG(val, msg) \
if(val == NULL) { ERR_MSG(msg);}

#define ERR_NULL(val) \
ERR_NULL_MSG(val, "")

//RETURNS if cond is false
#define ASSERT_MSG(cond, msg)\
if(!(cond)) { std::cerr << "ASSERT FAILED at file" __FILE__ " line " << __LINE__ << " : " msg << std::endl; __debugbreak(); return;}

#define ASSERT_MSG_V(cond, msg, ret) \
if(!(cond)) {std::cerr << "ASSERT FAILED at file" __FILE__ " line " << __LINE__ << " : " msg << std::endl; __debugbreak(); return ret;}

#define ASSERT(cond)\
if(!(cond)) { std::cerr << "ASSERT FAILED at file" __FILE__ " line " << __LINE__<< std::endl; __debugbreak(); return;}

#define ASSERT_V(cond, ret)\
if(!(cond)) { std::cerr << "ASSERT FAILED at file" __FILE__ " line " << __LINE__<< std::endl; __debugbreak(); return ret;}

#endif