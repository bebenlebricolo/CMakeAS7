if (NOT __AVR8-C)
  set (__AVR8-C 1)
else()
  return()
endif()

macro(__avr8_determine_compiler lang)
message("Determining C compiler for AVR 8 core")
  if (${lang} MATCHES C)
    get_filename_component(CMAKE_ATMEL_STUDIO_7_INSTALLATION_DIR
                           "[HKEY_CURRENT_USER\\Software\\Atmel\\AtmelStudio\\7.0_Config;InstallDir]/toolchain/avr8/avr8-gnu-toolchain/bin"
                           ABSOLUTE CACHE )
    #message ("CMAKE_ATMEL_STUDIO_7_INSTALLATION_DIR = ${CMAKE_ATMEL_STUDIO_7_INSTALLATION_DIR}")
    #if (EXISTS ${CMAKE_ATMEL_STUDIO_7_INSTALLATION_DIR})
    #  message("Verified that CMAKE_ATMEL_STUDIO_7_INSTALLATION_DIR is a directory")
    #endif()

    find_program(CMAKE_AVR_GCC_COMPILER NAMES avr-gcc.exe DOC "Atmel Studio AVR GCC" PATHS ${CMAKE_ATMEL_STUDIO_7_INSTALLATION_DIR} NO_DEFAULT_PATH )
    if ( NOT CMAKE_AVR_GCC_COMPILER )
      message( "Could not find avr-gcc executable in your Atmel Studio 7 installation path ")
    else()
      message( "Found avr-gcc compiler ! ")
      set ( CMAKE_C_COMPILER ${CMAKE_AVR_GCC_COMPILER} )
    endif()
  endif()

if(MSYS OR MINGW)
    # Create archiving rules to support large object file lists for static libraries.
    set(CMAKE_${lang}_ARCHIVE_CREATE "<CMAKE_AR> qc <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_${lang}_ARCHIVE_APPEND "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_${lang}_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")

    # Initialize C link type selection flags.  These flags are used when
    # building a shared library, shared module, or executable that links
    # to other libraries to select whether to use the static or shared
    # versions of the libraries.
    foreach(type SHARED_LIBRARY SHARED_MODULE EXE)
      set(CMAKE_${type}_LINK_STATIC_${lang}_FLAGS "-Wl,-Bstatic")
      set(CMAKE_${type}_LINK_DYNAMIC_${lang}_FLAGS "-Wl,-Bdynamic")
    endforeach()
  endif()

  # No -fPIC on AVR8
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIC "")
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIE "")
  set(_CMAKE_${lang}_PIE_MAY_BE_SUPPORTED_BY_LINKER NO)
  set(CMAKE_${lang}_LINK_OPTIONS_PIE "")
  set(CMAKE_${lang}_LINK_OPTIONS_NO_PIE "")
  set(CMAKE_SHARED_LIBRARY_${lang}_FLAGS "")

  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_OBJECTS ${__WINDOWS_GNU_LD_RESPONSE})
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_LIBRARIES ${__WINDOWS_GNU_LD_RESPONSE})
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_INCLUDES 1)

  # We prefer "@" for response files but it is not supported by gcc 3.
  execute_process(COMMAND ${CMAKE_${lang}_COMPILER} --version OUTPUT_VARIABLE _ver ERROR_VARIABLE _ver)
  if("${_ver}" MATCHES "\\(GCC\\) 3\\.")
    if("${lang}" STREQUAL "Fortran")
      # The GNU Fortran compiler reports an error:
      #   no input files; unwilling to write output files
      # when the response file is passed with "-Wl,@".
      set(CMAKE_Fortran_USE_RESPONSE_FILE_FOR_OBJECTS 0)
    else()
      # Use "-Wl,@" to pass the response file to the linker.
      set(CMAKE_${lang}_RESPONSE_FILE_LINK_FLAG "-Wl,@")
    endif()
    # The GNU 3.x compilers do not support response files (only linkers).
    set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_INCLUDES 0)
    # Link libraries are generated only for the front-end.
    set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_LIBRARIES 0)
  else()
    # Use "@" to pass the response file to the front-end.
    set(CMAKE_${lang}_RESPONSE_FILE_LINK_FLAG "@")
  endif()

  # Binary link rules.
  set(CMAKE_${lang}_CREATE_SHARED_MODULE
    "<CMAKE_${lang}_COMPILER> <CMAKE_SHARED_MODULE_${lang}_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_MODULE_CREATE_${lang}_FLAGS> -o <TARGET> ${CMAKE_GNULD_IMAGE_VERSION} <OBJECTS> <LINK_LIBRARIES>")
  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
    "<CMAKE_${lang}_COMPILER> <CMAKE_SHARED_LIBRARY_${lang}_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS> -o <TARGET> -Wl,--out-implib,<TARGET_IMPLIB> ${CMAKE_GNULD_IMAGE_VERSION} <OBJECTS> <LINK_LIBRARIES>")
  set(CMAKE_${lang}_LINK_EXECUTABLE
    "<CMAKE_${lang}_COMPILER> <FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> -Wl,--out-implib,<TARGET_IMPLIB> ${CMAKE_GNULD_IMAGE_VERSION} <LINK_LIBRARIES>")

  list(APPEND CMAKE_${lang}_ABI_FILES "Platform/Windows-GNU-${lang}-ABI")

  # Support very long lists of object files.
  # TODO: check for which gcc versions this is still needed, not needed for gcc >= 4.4.
  # Ninja generator doesn't support this work around.
  if("${CMAKE_${lang}_RESPONSE_FILE_LINK_FLAG}" STREQUAL "@" AND NOT CMAKE_GENERATOR MATCHES "Ninja")
    foreach(rule CREATE_SHARED_MODULE CREATE_SHARED_LIBRARY LINK_EXECUTABLE)
      # The gcc/collect2/ld toolchain does not use response files
      # internally so we cannot pass long object lists.  Instead pass
      # the object file list in a response file to the archiver to put
      # them in a temporary archive.  Hand the archive to the linker.
      string(REPLACE "<OBJECTS>" "-Wl,--whole-archive <OBJECT_DIR>/objects.a -Wl,--no-whole-archive"
        CMAKE_${lang}_${rule} "${CMAKE_${lang}_${rule}}")
      set(CMAKE_${lang}_${rule}
        "<CMAKE_COMMAND> -E rm -f <OBJECT_DIR>/objects.a"
        "<CMAKE_AR> cr <OBJECT_DIR>/objects.a <OBJECTS>"
        "${CMAKE_${lang}_${rule}}"
        )
    endforeach()
  endif()
endmacro()