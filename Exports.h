#pragma once

namespace MyUtilityLib
{
    #ifdef WIN32
      #include <winapifamily.h>
      #ifdef MYUTILITYLIB_EXPORTS
        #define EXP_MYUTILITYLIB __declspec(dllexport)
      #else
        #define EXP_MYUTILITYLIB __declspec(dllimport)
      #endif
      #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        #define DEPRECATED_MYUTILITYLIB __declspec(deprecated)
      #else
        #define DEPRECATED_MYUTILITYLIB
      #endif
      #pragma warning( disable: 4251 ) // shut up std::vector warnings
    #else
      #if __GNUC__ >= 4
        #define EXP_MYUTILITYLIB __attribute__ ((visibility("default")))
        #define DEPRECATED_MYUTILITYLIB __attribute__ ((__deprecated__))
      #else
        #define EXP_MYUTILITYLIB
        #define DEPRECATED_MYUTILITYLIB
      #endif
    #endif

    #define DISABLE_COPY(Class) \
        Class(const Class &) = delete; \
        Class &operator=(const Class &) = delete
}
