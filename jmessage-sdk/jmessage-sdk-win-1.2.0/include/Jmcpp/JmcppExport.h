#pragma once

#ifdef _MSC_VER

#ifdef JMCPP_EXPORTS
#define JMCPP_API __declspec(dllexport)
#else
#define JMCPP_API __declspec(dllimport)
#endif

#else

#define JMCPP_API __attribute__((visibility("default")))

#endif




