#pragma once

/**
 * Used for debug only
 * Contains asserts and other things stripped in release build
 */

#ifdef _DEBUG
#include <iostream>
#define ISF_ERROR(msg) std::cerr << msg << std::endl
/**
 * Throw an exception if the condition is not true
 */
#define ISF_ASSERT(x, msg) { if(!(x)) { ISF_ERROR(msg); __debugbreak(); } }
#else
#define ISF_ASSERT(x, msg)
#endif // DEBUG


