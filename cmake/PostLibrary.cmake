### Local override to set C_STANDARD for C code

if( NOT __PRE_LIBRARY_LOCAL__)
   set( __PRE_LIBRARY_LOCAL__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   #
   # Ensure C11 standard for max_align_t and other C11 features
   #
   if( LIBRARY_COMPILE_TARGET AND TARGET "${LIBRARY_COMPILE_TARGET}")
      set_target_properties( ${LIBRARY_COMPILE_TARGET} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)
   endif()
   if( LIBRARY_STAGE2_TARGET AND TARGET "${LIBRARY_STAGE2_TARGET}")
      set_target_properties( ${LIBRARY_STAGE2_TARGET} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)
   endif()

endif()