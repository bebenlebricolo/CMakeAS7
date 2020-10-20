if (__AVR8-C)
  return()
endif()

macro(__avr8_determine_compiler lang)

  if (${lang} MATCHES C)
    get_filename_component(CMAKE_ATMEL_STUDIO_7_AVR_8_TOOLCHAIN_DIR
                           "[HKEY_CURRENT_USER\\Software\\Atmel\\AtmelStudio\\7.0_Config;InstallDir]/toolchain/avr8/avr8-gnu-toolchain/bin"
                           ABSOLUTE CACHE )
    get_filename_component(CMAKE_ATMEL_STUDIO_7_INSTALLATION_DIR
                           "[HKEY_CURRENT_USER\\Software\\Atmel\\AtmelStudio\\7.0_Config;InstallDir]"
                           ABSOLUTE CACHE )
    find_program(CMAKE_AVR_GCC_COMPILER NAMES avr-gcc.exe DOC "Atmel Studio AVR GCC" PATHS ${CMAKE_ATMEL_STUDIO_7_AVR_8_TOOLCHAIN_DIR} NO_DEFAULT_PATH )
    find_program(CMAKE_ATMEL_STUDIO_EXECUTABLE NAMES AtmelStudio.exe DOC "Atmel Studio executable" PATHS ${CMAKE_ATMEL_STUDIO_7_INSTALLATION_DIR} NO_DEFAULT_PATH )
    if ( NOT CMAKE_AVR_GCC_COMPILER )
      message( "Could not find avr-gcc executable in your Atmel Studio 7 installation path ")
    else()
      set ( CMAKE_C_COMPILER ${CMAKE_AVR_GCC_COMPILER} )
    endif()
  endif()

  # No -fPIC on AVR8
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIC "")
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIE "")
  set(_CMAKE_${lang}_PIE_MAY_BE_SUPPORTED_BY_LINKER NO)
  set(CMAKE_${lang}_LINK_OPTIONS_PIE "")
  set(CMAKE_${lang}_LINK_OPTIONS_NO_PIE "")
  set(CMAKE_SHARED_LIBRARY_${lang}_FLAGS "")

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

  #list(APPEND CMAKE_${lang}_ABI_FILES "Platform/Windows-GNU-${lang}-ABI")

  # Flag which states "I've already processed all of the above!"
  set(__AVR8-C 1)
endmacro()