#ifdef NVE_BUILD_STATIC
    #define NVE_API extern
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #ifdef NVE_LIB
        #define NVE_API __declspec(dllexport)
    #else
        #define NVE_API __declspec(dllimport)
    #endif
#else
    #define NVE_API
#endif
