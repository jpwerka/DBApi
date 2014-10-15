#undef SGLIB_EXPORTS
#undef SGLIB_USEDEF
#ifndef SGLIB_IMPORTS
   #define SGLIB_EXPORTS
   //#define SGLIB_USEDEF
#endif

#undef SGAPI
#ifdef SG_WINDOWS
   #ifdef SGLIB_EXPORTS
      #ifdef SGLIB_USEDEF
         #define SGAPI
         #pragma message("Utilizando " __APPLIB__ " com def file")
      #else 
         #define SGAPI __declspec(dllexport)
         #pragma message("Utilizando " __APPLIB__ " com dllexport")
      #endif   
   #else
      #define SGAPI __declspec(dllimport)
      #pragma message("Utilizando " __APPLIB__ " com dllimport")
   #endif
   #define SGENTRY __cdecl
#else
   #define SGAPI
   #define SGENTRY
#endif
