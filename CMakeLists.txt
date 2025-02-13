# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.1...3.20 FATAL_ERROR)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_C ${CMAKE_CURRENT_SOURCE_DIR}/Source/Modules/OverrideC.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/Source/Modules/OverrideCXX.cmake)
project(CMake)
unset(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX)
unset(CMAKE_USER_MAKE_RULES_OVERRIDE_C)

# FIXME: This block should go away after a transition period.
if(MSVC AND NOT CMAKE_VERSION VERSION_LESS 3.15)
  # Filter out MSVC runtime library flags that may have come from
  # the cache of an existing build tree or from scripts.
  foreach(l C CXX)
    foreach(c DEBUG MINSIZEREL RELEASE RELWITHDEBINFO)
      string(REGEX REPLACE "[-/]M[DT]d?( |$)" "" "CMAKE_${l}_FLAGS_${c}" "${CMAKE_${l}_FLAGS_${c}}")
    endforeach()
  endforeach()
endif()

# Make sure we can find internal find_package modules only used for
# building CMake and not for shipping externally
list(INSERT CMAKE_MODULE_PATH 0 ${CMake_SOURCE_DIR}/Source/Modules)

if(CMAKE_BOOTSTRAP)
  # Running from bootstrap script.  Set local variable and remove from cache.
  set(CMAKE_BOOTSTRAP 1)
  unset(CMAKE_BOOTSTRAP CACHE)
endif()

if(CMake_TEST_HOST_CMAKE)
  get_filename_component(CMake_TEST_EXTERNAL_CMAKE "${CMAKE_COMMAND}" DIRECTORY)
endif()

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  if(CMAKE_SYSTEM_NAME STREQUAL "HP-UX")
    message(FATAL_ERROR
      "CMake no longer compiles on HP-UX.  See\n"
      "  https://gitlab.kitware.com/cmake/cmake/-/issues/17137\n"
      "Use CMake 3.9 or lower instead."
      )
  endif()

  set(CMake_BIN_DIR ${CMake_BINARY_DIR}/bin)
endif()

if(CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL)
  if(CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL MATCHES "^3|2\\.1$")
    set(USE_LGPL "${CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL}")
  else()
    set(USE_LGPL "2.1")
  endif()
else()
  set(USE_LGPL "")
endif()

if("${CMake_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  # Disallow architecture-specific try_run.  It may not run on the host.
  macro(TRY_RUN)
    if(CMAKE_TRY_COMPILE_OSX_ARCHITECTURES)
      message(FATAL_ERROR "TRY_RUN not allowed with CMAKE_TRY_COMPILE_OSX_ARCHITECTURES=[${CMAKE_TRY_COMPILE_OSX_ARCHITECTURES}]")
    else()
      _TRY_RUN(${ARGV})
    endif()
  endmacro()
endif()

# Use most-recent available language dialects with GNU and Clang
if(NOT DEFINED CMAKE_C_STANDARD AND NOT CMake_NO_C_STANDARD)
  include(${CMake_SOURCE_DIR}/Source/Checks/cm_c11_thread_local.cmake)
  if(NOT CMake_C11_THREAD_LOCAL_BROKEN)
    set(CMAKE_C_STANDARD 11)
  else()
    set(CMAKE_C_STANDARD 99)
  endif()
endif()
if(NOT DEFINED CMAKE_CXX_STANDARD AND NOT CMake_NO_CXX_STANDARD)
  if (CMAKE_CXX_COMPILER_ID STREQUAL SunPro AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.14)
    set(CMAKE_CXX_STANDARD 98)
  else()
    if(NOT CMAKE_VERSION VERSION_LESS 3.8)
      include(${CMake_SOURCE_DIR}/Source/Checks/cm_cxx17_check.cmake)
    else()
      set(CMake_CXX17_BROKEN 1)
    endif()
    if(NOT CMake_CXX17_BROKEN)
      set(CMAKE_CXX_STANDARD 17)
    else()
      include(${CMake_SOURCE_DIR}/Source/Checks/cm_cxx14_check.cmake)
      if(NOT CMake_CXX14_BROKEN)
        set(CMAKE_CXX_STANDARD 14)
      else()
        set(CMAKE_CXX_STANDARD 11)
      endif()
    endif()
  endif()
endif()
if(NOT CMake_TEST_EXTERNAL_CMAKE)
  # include special compile flags for some compilers
  include(CompileFlags.cmake)

  # check for available C++ features
  include(${CMake_SOURCE_DIR}/Source/Checks/cm_cxx_features.cmake)

  if(NOT CMake_HAVE_CXX_UNIQUE_PTR)
    message(FATAL_ERROR "The C++ compiler does not support C++11 (e.g. std::unique_ptr).")
  endif()
endif()

# Inform STL library header wrappers whether to use system versions.
configure_file(${CMake_SOURCE_DIR}/Utilities/std/cmSTL.hxx.in
  ${CMake_BINARY_DIR}/Utilities/cmSTL.hxx
  @ONLY)

# set the internal encoding of CMake to UTF-8
set(KWSYS_ENCODING_DEFAULT_CODEPAGE CP_UTF8)

# option to use COMPONENT with install command
option(CMake_INSTALL_COMPONENTS "Using components when installing" OFF)
mark_as_advanced(CMake_INSTALL_COMPONENTS)
macro(CMake_OPTIONAL_COMPONENT NAME)
  if(CMake_INSTALL_COMPONENTS)
    set(COMPONENT COMPONENT ${NAME})
  else()
    set(COMPONENT)
  endif()
endmacro()

# option to disable installing 3rd-party dependencies
option(CMake_INSTALL_DEPENDENCIES
  "Whether to install 3rd-party runtime dependencies" OFF)
mark_as_advanced(CMake_INSTALL_DEPENDENCIES)

# option to build reference for CMake developers
option(CMake_BUILD_DEVELOPER_REFERENCE
  "Build CMake Developer Reference" OFF)
mark_as_advanced(CMake_BUILD_DEVELOPER_REFERENCE)

# option to build using interprocedural optimizations (IPO/LTO)
if (NOT CMAKE_VERSION VERSION_LESS 3.12.2)
  option(CMake_BUILD_LTO "Compile CMake with link-time optimization if supported" OFF)
  if(CMake_BUILD_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT HAVE_IPO)
    if(HAVE_IPO)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
  endif()
endif()

#-----------------------------------------------------------------------
# a macro to deal with system libraries, implemented as a macro
# simply to improve readability of the main script
#-----------------------------------------------------------------------
macro(CMAKE_HANDLE_SYSTEM_LIBRARIES)
  # Options have dependencies.
  include(CMakeDependentOption)

  # Allow the user to enable/disable all system utility library options by
  # defining CMAKE_USE_SYSTEM_LIBRARIES or CMAKE_USE_SYSTEM_LIBRARY_${util}.
  set(UTILITIES BZIP2 CURL EXPAT FORM JSONCPP LIBARCHIVE LIBLZMA LIBRHASH LIBUV NGHTTP2 ZLIB ZSTD)
  foreach(util ${UTILITIES})
    if(NOT DEFINED CMAKE_USE_SYSTEM_LIBRARY_${util}
        AND DEFINED CMAKE_USE_SYSTEM_LIBRARIES)
      set(CMAKE_USE_SYSTEM_LIBRARY_${util} "${CMAKE_USE_SYSTEM_LIBRARIES}")
    endif()
    if(DEFINED CMAKE_USE_SYSTEM_LIBRARY_${util})
      if(CMAKE_USE_SYSTEM_LIBRARY_${util})
        set(CMAKE_USE_SYSTEM_LIBRARY_${util} ON)
      else()
        set(CMAKE_USE_SYSTEM_LIBRARY_${util} OFF)
      endif()
      if(CMAKE_BOOTSTRAP)
        unset(CMAKE_USE_SYSTEM_LIBRARY_${util} CACHE)
      endif()
      string(TOLOWER "${util}" lutil)
      set(CMAKE_USE_SYSTEM_${util} "${CMAKE_USE_SYSTEM_LIBRARY_${util}}"
        CACHE BOOL "Use system-installed ${lutil}" FORCE)
    else()
      set(CMAKE_USE_SYSTEM_LIBRARY_${util} OFF)
    endif()
  endforeach()
  if(CMAKE_BOOTSTRAP)
    unset(CMAKE_USE_SYSTEM_LIBRARIES CACHE)
  endif()

  # Optionally use system utility libraries.
  option(CMAKE_USE_SYSTEM_LIBARCHIVE "Use system-installed libarchive" "${CMAKE_USE_SYSTEM_LIBRARY_LIBARCHIVE}")
  option(CMAKE_USE_SYSTEM_CURL "Use system-installed curl" "${CMAKE_USE_SYSTEM_LIBRARY_CURL}")
  option(CMAKE_USE_SYSTEM_EXPAT "Use system-installed expat" "${CMAKE_USE_SYSTEM_LIBRARY_EXPAT}")
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_ZLIB "Use system-installed zlib"
    "${CMAKE_USE_SYSTEM_LIBRARY_ZLIB}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE;NOT CMAKE_USE_SYSTEM_CURL" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_BZIP2 "Use system-installed bzip2"
    "${CMAKE_USE_SYSTEM_LIBRARY_BZIP2}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_ZSTD "Use system-installed zstd"
    "${CMAKE_USE_SYSTEM_LIBRARY_ZSTD}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_LIBLZMA "Use system-installed liblzma"
    "${CMAKE_USE_SYSTEM_LIBRARY_LIBLZMA}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_NGHTTP2 "Use system-installed nghttp2"
    "${CMAKE_USE_SYSTEM_LIBRARY_NGHTTP2}" "NOT CMAKE_USE_SYSTEM_CURL" ON)
  option(CMAKE_USE_SYSTEM_FORM "Use system-installed libform" "${CMAKE_USE_SYSTEM_LIBRARY_FORM}")
  option(CMAKE_USE_SYSTEM_JSONCPP "Use system-installed jsoncpp" "${CMAKE_USE_SYSTEM_LIBRARY_JSONCPP}")
  option(CMAKE_USE_SYSTEM_LIBRHASH "Use system-installed librhash" "${CMAKE_USE_SYSTEM_LIBRARY_LIBRHASH}")
  option(CMAKE_USE_SYSTEM_LIBUV "Use system-installed libuv" "${CMAKE_USE_SYSTEM_LIBRARY_LIBUV}")

  # For now use system KWIML only if explicitly requested rather
  # than activating via the general system libs options.
  option(CMAKE_USE_SYSTEM_KWIML "Use system-installed KWIML" OFF)
  mark_as_advanced(CMAKE_USE_SYSTEM_KWIML)

  # Mention to the user what system libraries are being used.
  foreach(util ${UTILITIES} KWIML)
    if(CMAKE_USE_SYSTEM_${util})
      message(STATUS "Using system-installed ${util}")
    endif()
  endforeach()

  # Inform utility library header wrappers whether to use system versions.
  configure_file(${CMake_SOURCE_DIR}/Utilities/cmThirdParty.h.in
    ${CMake_BINARY_DIR}/Utilities/cmThirdParty.h
    @ONLY)

endmacro()

#-----------------------------------------------------------------------
# a macro to determine the generator and ctest executable to use
# for testing. Simply to improve readability of the main script.
#-----------------------------------------------------------------------
macro(CMAKE_SETUP_TESTING)
  if(BUILD_TESTING)
    set(CMAKE_TEST_SYSTEM_LIBRARIES 0)
    foreach(util CURL EXPAT ZLIB)
      if(CMAKE_USE_SYSTEM_${util})
        set(CMAKE_TEST_SYSTEM_LIBRARIES 1)
      endif()
    endforeach()

    # This variable is set by cmake, however to
    # test cmake we want to make sure that
    # the ctest from this cmake is used for testing
    # and not the ctest from the cmake building and testing
    # cmake.
    if(CMake_TEST_EXTERNAL_CMAKE)
      set(CMAKE_CTEST_COMMAND "${CMake_TEST_EXTERNAL_CMAKE}/ctest")
      set(CMAKE_CMAKE_COMMAND "${CMake_TEST_EXTERNAL_CMAKE}/cmake")
      set(CMAKE_CPACK_COMMAND "${CMake_TEST_EXTERNAL_CMAKE}/cpack")
      foreach(exe cmake ctest cpack)
        add_executable(${exe} IMPORTED)
        set_property(TARGET ${exe} PROPERTY IMPORTED_LOCATION ${CMake_TEST_EXTERNAL_CMAKE}/${exe})
      endforeach()
    else()
      set(CMAKE_CTEST_COMMAND "${CMake_BIN_DIR}/ctest")
      set(CMAKE_CMAKE_COMMAND "${CMake_BIN_DIR}/cmake")
      set(CMAKE_CPACK_COMMAND "${CMake_BIN_DIR}/cpack")
    endif()
  endif()

  # configure some files for testing
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Templates/CTestScript.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CTestScript.cmake"
    @ONLY)
  configure_file(${CMake_SOURCE_DIR}/Tests/.NoDartCoverage
    ${CMake_BINARY_DIR}/Tests/.NoDartCoverage)
  configure_file(${CMake_SOURCE_DIR}/Tests/.NoDartCoverage
    ${CMake_BINARY_DIR}/Modules/.NoDartCoverage)
  configure_file(${CMake_SOURCE_DIR}/CTestCustom.cmake.in
    ${CMake_BINARY_DIR}/CTestCustom.cmake @ONLY)
  if(BUILD_TESTING AND DART_ROOT)
    configure_file(${CMake_SOURCE_DIR}/CMakeLogo.gif
      ${CMake_BINARY_DIR}/Testing/HTML/TestingResults/Icons/Logo.gif COPYONLY)
  endif()
  mark_as_advanced(DART_ROOT)
endmacro()


# Provide a way for Visual Studio Express users to turn OFF the new FOLDER
# organization feature. Default to ON for non-Express users. Express users must
# explicitly turn off this option to build CMake in the Express IDE...
#
option(CMAKE_USE_FOLDERS "Enable folder grouping of projects in IDEs." ON)
mark_as_advanced(CMAKE_USE_FOLDERS)


option(CMake_RUN_CLANG_TIDY "Run clang-tidy with the compiler." OFF)
if(CMake_RUN_CLANG_TIDY)
  if(CMake_SOURCE_DIR STREQUAL CMake_BINARY_DIR)
    message(FATAL_ERROR "CMake_RUN_CLANG_TIDY requires an out-of-source build!")
  endif()
  find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
  if(NOT CLANG_TIDY_COMMAND)
    message(FATAL_ERROR "CMake_RUN_CLANG_TIDY is ON but clang-tidy is not found!")
  endif()
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")

  # Create a preprocessor definition that depends on .clang-tidy content so
  # the compile command will change when .clang-tidy changes.  This ensures
  # that a subsequent build re-runs clang-tidy on all sources even if they
  # do not otherwise need to be recompiled.  Nothing actually uses this
  # definition.  We add it to targets on which we run clang-tidy just to
  # get the build dependency on the .clang-tidy file.
  file(SHA1 ${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy clang_tidy_sha1)
  set(CLANG_TIDY_DEFINITIONS "CLANG_TIDY_SHA1=${clang_tidy_sha1}")
  unset(clang_tidy_sha1)

endif()
configure_file(.clang-tidy .clang-tidy COPYONLY)


option(CMake_RUN_IWYU "Run include-what-you-use with the compiler." OFF)
if(CMake_RUN_IWYU)
  find_program(IWYU_COMMAND NAMES include-what-you-use iwyu)
  if(NOT IWYU_COMMAND)
    message(FATAL_ERROR "CMake_RUN_IWYU is ON but include-what-you-use is not found!")
  endif()
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
    "${IWYU_COMMAND};-Xiwyu;--mapping_file=${CMake_SOURCE_DIR}/Utilities/IWYU/mapping.imp;-w")
  list(APPEND CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${CMake_IWYU_OPTIONS})
endif()


#-----------------------------------------------------------------------
# a macro that only sets the FOLDER target property if it's
# "appropriate"
#-----------------------------------------------------------------------
macro(CMAKE_SET_TARGET_FOLDER tgt folder)
  if(CMAKE_USE_FOLDERS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    if(MSVC AND TARGET ${tgt})
      set_property(TARGET "${tgt}" PROPERTY FOLDER "${folder}")
    endif()
  else()
    set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
  endif()
endmacro()


#-----------------------------------------------------------------------
# a macro to build the utilities used by CMake
# Simply to improve readability of the main script.
#-----------------------------------------------------------------------
macro (CMAKE_BUILD_UTILITIES)
  find_package(Threads)

  # Suppress unnecessary checks in third-party code.
  include(Utilities/cmThirdPartyChecks.cmake)

  #---------------------------------------------------------------------
  # Create the kwsys library for CMake.
  set(KWSYS_NAMESPACE cmsys)
  set(KWSYS_USE_SystemTools 1)
  set(KWSYS_USE_Directory 1)
  set(KWSYS_USE_RegularExpression 1)
  set(KWSYS_USE_Base64 1)
  set(KWSYS_USE_MD5 1)
  set(KWSYS_USE_Process 1)
  set(KWSYS_USE_CommandLineArguments 1)
  set(KWSYS_USE_ConsoleBuf 1)
  set(KWSYS_HEADER_ROOT ${CMake_BINARY_DIR}/Source)
  set(KWSYS_INSTALL_DOC_DIR "${CMAKE_DOC_DIR}")
  if(CMake_NO_CXX_STANDARD)
    set(KWSYS_CXX_STANDARD "")
  endif()
  if(CMake_NO_SELF_BACKTRACE)
    set(KWSYS_NO_EXECINFO 1)
  endif()
  if(WIN32)
    # FIXME: Teach KWSys to hard-code these checks on Windows.
    set(KWSYS_C_HAS_CLOCK_GETTIME_MONOTONIC_COMPILED 0)
    set(KWSYS_C_HAS_PTRDIFF_T_COMPILED 1)
    set(KWSYS_CXX_HAS_ENVIRON_IN_STDLIB_H_COMPILED 1)
    set(KWSYS_CXX_HAS_RLIMIT64_COMPILED 0)
    set(KWSYS_CXX_HAS_SETENV_COMPILED 0)
    set(KWSYS_CXX_HAS_UNSETENV_COMPILED 0)
    set(KWSYS_CXX_HAS_UTIMENSAT_COMPILED 0)
    set(KWSYS_CXX_HAS_UTIMES_COMPILED 0)
    set(KWSYS_CXX_STAT_HAS_ST_MTIM_COMPILED 0)
    set(KWSYS_CXX_STAT_HAS_ST_MTIMESPEC_COMPILED 0)
    set(KWSYS_STL_HAS_WSTRING_COMPILED 1)
    set(KWSYS_SYS_HAS_IFADDRS_H 0)
  endif()
  add_subdirectory(Source/kwsys)
  set(kwsys_folder "Utilities/KWSys")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE} "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}_c "${kwsys_folder}")
  if(BUILD_TESTING)
    CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestDynload "${kwsys_folder}")
    CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestProcess "${kwsys_folder}")
    CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestsC "${kwsys_folder}")
    CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestsCxx "${kwsys_folder}")
    CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestSharedForward "${kwsys_folder}")
  endif()

  #---------------------------------------------------------------------
  # Setup third-party libraries.
  # Everything in the tree should be able to include files from the
  # Utilities directory.
  if ((CMAKE_SYSTEM_NAME STREQUAL "AIX" OR CMAKE_SYSTEM_NAME STREQUAL "OS400") AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using -isystem option generate error "template with C linkage"
    include_directories("${CMake_SOURCE_DIR}/Utilities/std")
  else()
    include_directories(SYSTEM "${CMake_SOURCE_DIR}/Utilities/std")
  endif()

  include_directories("${CMake_BINARY_DIR}/Utilities")
  if ((CMAKE_SYSTEM_NAME STREQUAL "AIX" OR CMAKE_SYSTEM_NAME STREQUAL "OS400") AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using -isystem option generate error "template with C linkage"
    include_directories("${CMake_SOURCE_DIR}/Utilities")
  else()
    include_directories(SYSTEM "${CMake_SOURCE_DIR}/Utilities")
  endif()

  #---------------------------------------------------------------------
  # Build CMake std library for CMake and CTest.
  set(CMAKE_STD_LIBRARY cmstd)
  add_subdirectory(Utilities/std)
  CMAKE_SET_TARGET_FOLDER(cmstd "Utilities/std")

  # check for the use of system libraries versus builtin ones
  # (a macro defined in this file)
  CMAKE_HANDLE_SYSTEM_LIBRARIES()

  if(CMAKE_USE_SYSTEM_KWIML)
    find_package(KWIML 1.0)
    if(NOT KWIML_FOUND)
      message(FATAL_ERROR "CMAKE_USE_SYSTEM_KWIML is ON but KWIML is not found!")
    endif()
    set(CMake_KWIML_LIBRARIES kwiml::kwiml)
  else()
    set(CMake_KWIML_LIBRARIES "")
    if(BUILD_TESTING)
      set(KWIML_TEST_ENABLE 1)
    endif()
    add_subdirectory(Utilities/KWIML)
  endif()

  if(CMAKE_USE_SYSTEM_LIBRHASH)
    find_package(LibRHash)
    if(NOT LibRHash_FOUND)
      message(FATAL_ERROR
        "CMAKE_USE_SYSTEM_LIBRHASH is ON but LibRHash is not found!")
    endif()
    set(CMAKE_LIBRHASH_LIBRARIES LibRHash::LibRHash)
  else()
    set(CMAKE_LIBRHASH_LIBRARIES cmlibrhash)
    add_subdirectory(Utilities/cmlibrhash)
    CMAKE_SET_TARGET_FOLDER(cmlibrhash "Utilities/3rdParty")
  endif()

  #---------------------------------------------------------------------
  # Build zlib library for Curl, CMake, and CTest.
  set(CMAKE_ZLIB_HEADER "cm_zlib.h")
  if(CMAKE_USE_SYSTEM_ZLIB)
    find_package(ZLIB)
    if(NOT ZLIB_FOUND)
      message(FATAL_ERROR
        "CMAKE_USE_SYSTEM_ZLIB is ON but a zlib is not found!")
    endif()
    set(CMAKE_ZLIB_INCLUDES ${ZLIB_INCLUDE_DIR})
    set(CMAKE_ZLIB_LIBRARIES ${ZLIB_LIBRARIES})
  else()
    set(CMAKE_ZLIB_INCLUDES ${CMake_SOURCE_DIR}/Utilities)
    set(CMAKE_ZLIB_LIBRARIES cmzlib)
    set(WITHOUT_ZLIB_DLL "")
    set(WITHOUT_ZLIB_DLL_WITH_LIB cmzlib)
    set(ZLIB_DLL "")
    set(ZLIB_DLL_WITH_LIB cmzlib)
    set(ZLIB_WINAPI "")
    set(ZLIB_WINAPI_COMPILED 0)
    set(ZLIB_WINAPI_WITH_LIB cmzlib)
    add_subdirectory(Utilities/cmzlib)
    CMAKE_SET_TARGET_FOLDER(cmzlib "Utilities/3rdParty")
  endif()

  #---------------------------------------------------------------------
  # Build Curl library for CTest.
  if(CMAKE_USE_SYSTEM_CURL)
    find_package(CURL)
    if(NOT CURL_FOUND)
      message(FATAL_ERROR
        "CMAKE_USE_SYSTEM_CURL is ON but a curl is not found!")
    endif()
    set(CMAKE_CURL_INCLUDES ${CURL_INCLUDE_DIRS})
    set(CMAKE_CURL_LIBRARIES ${CURL_LIBRARIES})
  else()
    set(CURL_SPECIAL_ZLIB_H ${CMAKE_ZLIB_HEADER})
    set(CURL_SPECIAL_LIBZ_INCLUDES ${CMAKE_ZLIB_INCLUDES})
    set(CURL_SPECIAL_LIBZ ${CMAKE_ZLIB_LIBRARIES})
    set(CMAKE_CURL_INCLUDES)
    set(CMAKE_CURL_LIBRARIES cmcurl)
    if(CMAKE_TESTS_CDASH_SERVER)
      set(CMAKE_CURL_TEST_URL "${CMAKE_TESTS_CDASH_SERVER}/user.php")
    endif()
    set(_CMAKE_USE_OPENSSL_DEFAULT OFF)
    if(NOT DEFINED CMAKE_USE_OPENSSL AND NOT WIN32 AND NOT APPLE
        AND CMAKE_SYSTEM_NAME MATCHES "(Linux|FreeBSD)")
      set(_CMAKE_USE_OPENSSL_DEFAULT ON)
    endif()
    option(CMAKE_USE_OPENSSL "Use OpenSSL." ${_CMAKE_USE_OPENSSL_DEFAULT})
    mark_as_advanced(CMAKE_USE_OPENSSL)
    if(CMAKE_USE_OPENSSL)
      set(CURL_CA_BUNDLE "" CACHE FILEPATH "Path to SSL CA Certificate Bundle")
      set(CURL_CA_PATH "" CACHE PATH "Path to SSL CA Certificate Directory")
      mark_as_advanced(CURL_CA_BUNDLE CURL_CA_PATH)
    endif()
    if(NOT CMAKE_USE_SYSTEM_NGHTTP2)
      # Tell curl's FindNGHTTP2 module to use our library.
      set(NGHTTP2_LIBRARY cmnghttp2)
      set(NGHTTP2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmnghttp2/lib/includes)
    endif()
    add_subdirectory(Utilities/cmcurl)
    CMAKE_SET_TARGET_FOLDER(cmcurl "Utilities/3rdParty")
    CMAKE_SET_TARGET_FOLDER(LIBCURL "Utilities/3rdParty")
    if(NOT CMAKE_USE_SYSTEM_NGHTTP2)
      # Configure after curl to re-use some check results.
      add_subdirectory(Utilities/cmnghttp2)
      CMAKE_SET_TARGET_FOLDER(cmnghttp2 "Utilities/3rdParty")
    endif()
  endif()

  #---------------------------------------------------------------------
  # Build expat library for CMake, CTest, and libarchive.
  if(CMAKE_USE_SYSTEM_EXPAT)
    find_package(EXPAT)
    if(NOT EXPAT_FOUND)
      message(FATAL_ERROR
        "CMAKE_USE_SYSTEM_EXPAT is ON but a expat is not found!")
    endif()
    set(CMAKE_EXPAT_INCLUDES ${EXPAT_INCLUDE_DIRS})
    set(CMAKE_EXPAT_LIBRARIES ${EXPAT_LIBRARIES})
  else()
    set(CMAKE_EXPAT_INCLUDES)
    set(CMAKE_EXPAT_LIBRARIES cmexpat)
    add_subdirectory(Utilities/cmexpat)
    CMAKE_SET_TARGET_FOLDER(cmexpat "Utilities/3rdParty")
  endif()

  #---------------------------------------------------------------------
  # Build or use system libbz2 for libarchive.
  if(NOT CMAKE_USE_SYSTEM_LIBARCHIVE)
    if(CMAKE_USE_SYSTEM_BZIP2)
      find_package(BZip2)
    else()
      set(BZIP2_INCLUDE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmbzip2")
      set(BZIP2_LIBRARIES cmbzip2)
      set(BZIP2_NEED_PREFIX "")
      set(USE_BZIP2_DLL "")
      set(USE_BZIP2_DLL_WITH_LIB cmbzip2)
      set(USE_BZIP2_STATIC "")
      set(USE_BZIP2_STATIC_WITH_LIB cmbzip2)
      add_subdirectory(Utilities/cmbzip2)
      CMAKE_SET_TARGET_FOLDER(cmbzip2 "Utilities/3rdParty")
    endif()
  endif()

  #---------------------------------------------------------------------
  # Build or use system zstd for libarchive.
  if(NOT CMAKE_USE_SYSTEM_LIBARCHIVE)
    if(NOT CMAKE_USE_SYSTEM_ZSTD)
      set(ZSTD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmzstd")
      set(ZSTD_LIBRARY cmzstd)
      add_subdirectory(Utilities/cmzstd)
      CMAKE_SET_TARGET_FOLDER(cmzstd "Utilities/3rdParty")
    endif()
  endif()

  #---------------------------------------------------------------------
  # Build or use system liblzma for libarchive.
  if(NOT CMAKE_USE_SYSTEM_LIBARCHIVE)
    if(CMAKE_USE_SYSTEM_LIBLZMA)
      find_package(LibLZMA)
      if(NOT LIBLZMA_FOUND)
        message(FATAL_ERROR "CMAKE_USE_SYSTEM_LIBLZMA is ON but LibLZMA is not found!")
      endif()
    else()
      add_subdirectory(Utilities/cmliblzma)
      CMAKE_SET_TARGET_FOLDER(cmliblzma "Utilities/3rdParty")
      set(LIBLZMA_HAS_AUTO_DECODER 1)
      set(LIBLZMA_HAS_EASY_ENCODER 1)
      set(LIBLZMA_HAS_LZMA_PRESET 1)
      set(LIBLZMA_INCLUDE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmliblzma/liblzma/api")
      set(LIBLZMA_LIBRARY cmliblzma)
      set(HAVE_LZMA_STREAM_ENCODER_MT 1)
    endif()
  endif()

  #---------------------------------------------------------------------
  # Build or use system libarchive for CMake and CTest.
  if(CMAKE_USE_SYSTEM_LIBARCHIVE)
    find_package(LibArchive 3.3.3)
    if(NOT LibArchive_FOUND)
      message(FATAL_ERROR "CMAKE_USE_SYSTEM_LIBARCHIVE is ON but LibArchive is not found!")
    endif()
    set(CMAKE_TAR_INCLUDES ${LibArchive_INCLUDE_DIRS})
    set(CMAKE_TAR_LIBRARIES ${LibArchive_LIBRARIES})
  else()
    set(EXPAT_INCLUDE_DIR ${CMAKE_EXPAT_INCLUDES})
    set(EXPAT_LIBRARY ${CMAKE_EXPAT_LIBRARIES})
    set(ZLIB_INCLUDE_DIR ${CMAKE_ZLIB_INCLUDES})
    set(ZLIB_LIBRARY ${CMAKE_ZLIB_LIBRARIES})
    add_definitions(-DLIBARCHIVE_STATIC)
    set(ENABLE_MBEDTLS OFF CACHE INTERNAL "Enable use of mbed TLS")
    set(ENABLE_NETTLE OFF CACHE INTERNAL "Enable use of Nettle")
    set(ENABLE_OPENSSL ${CMAKE_USE_OPENSSL} CACHE INTERNAL "Enable use of OpenSSL")
    set(ENABLE_LIBB2 OFF CACHE INTERNAL "Enable the use of the system LIBB2 library if found")
    set(ENABLE_LZMA ON CACHE INTERNAL "Enable the use of the system LZMA library if found")
    set(ENABLE_LZ4 OFF CACHE INTERNAL "Enable the use of the system LZ4 library if found")
    set(ENABLE_LZO OFF CACHE INTERNAL "Enable the use of the system LZO library if found")
    set(ENABLE_ZLIB ON CACHE INTERNAL "Enable the use of the system ZLIB library if found")
    set(ENABLE_BZip2 ON CACHE INTERNAL "Enable the use of the system BZip2 library if found")
    set(ENABLE_ZSTD ON CACHE INTERNAL "Enable the use of the system zstd library if found")
    set(ENABLE_LIBXML2 OFF CACHE INTERNAL "Enable the use of the system libxml2 library if found")
    set(ENABLE_EXPAT OFF CACHE INTERNAL "Enable the use of the system EXPAT library if found")
    set(ENABLE_PCREPOSIX OFF CACHE INTERNAL "Enable the use of the system PCREPOSIX library if found")
    set(ENABLE_LibGCC OFF CACHE INTERNAL "Enable the use of the system LibGCC library if found")
    set(ENABLE_XATTR OFF CACHE INTERNAL "Enable extended attribute support")
    set(ENABLE_ACL OFF CACHE INTERNAL "Enable ACL support")
    set(ENABLE_ICONV OFF CACHE INTERNAL "Enable iconv support")
    set(ENABLE_CNG OFF CACHE INTERNAL "Enable the use of CNG(Crypto Next Generation)")
    SET(POSIX_REGEX_LIB "" CACHE INTERNAL "Choose what library should provide POSIX regular expression support")
    add_subdirectory(Utilities/cmlibarchive)
    CMAKE_SET_TARGET_FOLDER(cmlibarchive "Utilities/3rdParty")
    set(CMAKE_TAR_LIBRARIES cmlibarchive ${BZIP2_LIBRARIES})
  endif()

  #---------------------------------------------------------------------
  # Build jsoncpp library.
  if(CMAKE_USE_SYSTEM_JSONCPP)
    find_package(JsonCpp 1.4.1)
    if(NOT JsonCpp_FOUND)
      message(FATAL_ERROR
        "CMAKE_USE_SYSTEM_JSONCPP is ON but a JsonCpp is not found!")
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
      set_property(TARGET JsonCpp::JsonCpp APPEND PROPERTY
        INTERFACE_COMPILE_OPTIONS -Wno-deprecated-declarations)
    endif()
    set(CMAKE_JSONCPP_LIBRARIES JsonCpp::JsonCpp)
  else()
    set(CMAKE_JSONCPP_LIBRARIES cmjsoncpp)
    add_subdirectory(Utilities/cmjsoncpp)
    CMAKE_SET_TARGET_FOLDER(cmjsoncpp "Utilities/3rdParty")
  endif()

  #---------------------------------------------------------------------
  # Build libuv library.
  if(CMAKE_USE_SYSTEM_LIBUV)
    if(WIN32)
      find_package(LibUV 1.38.0)
    else()
      find_package(LibUV 1.10.0)
    endif()
    if(NOT LIBUV_FOUND)
      message(FATAL_ERROR
        "CMAKE_USE_SYSTEM_LIBUV is ON but a libuv is not found!")
    endif()
    set(CMAKE_LIBUV_LIBRARIES LibUV::LibUV)
  else()
    set(CMAKE_LIBUV_LIBRARIES cmlibuv)
    add_subdirectory(Utilities/cmlibuv)
    CMAKE_SET_TARGET_FOLDER(cmlibuv "Utilities/3rdParty")
  endif()

  #---------------------------------------------------------------------
  # Use curses?
  if (UNIX)
    if(NOT DEFINED BUILD_CursesDialog)
      include(${CMake_SOURCE_DIR}/Source/Checks/Curses.cmake)
      option(BUILD_CursesDialog "Build the CMake Curses Dialog ccmake" "${CMakeCheckCurses_COMPILED}")
    endif()
  else ()
    set(BUILD_CursesDialog 0)
  endif ()
  if(BUILD_CursesDialog)
    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses)
    if(NOT CURSES_FOUND)
      message(WARNING
        "'ccmake' will not be built because Curses was not found.\n"
        "Turn off BUILD_CursesDialog to suppress this message."
        )
      set(BUILD_CursesDialog 0)
    endif()
  endif()
  if(BUILD_CursesDialog)
    if(NOT CMAKE_USE_SYSTEM_FORM)
      add_subdirectory(Source/CursesDialog/form)
    elseif(NOT CURSES_FORM_LIBRARY)
      message( FATAL_ERROR "CMAKE_USE_SYSTEM_FORM in ON but CURSES_FORM_LIBRARY is not set!" )
    endif()
  endif()
endmacro ()

#-----------------------------------------------------------------------
if(NOT CMake_TEST_EXTERNAL_CMAKE)
  if(CMAKE_CXX_PLATFORM_ID MATCHES "OpenBSD")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER}
      ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
      OUTPUT_VARIABLE _GXX_VERSION
    )
    string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1\\2"
      _GXX_VERSION_SHORT ${_GXX_VERSION})
    if(_GXX_VERSION_SHORT EQUAL 33)
      message(FATAL_ERROR
        "GXX 3.3 on OpenBSD is known to cause CPack to Crash.\n"
        "Please use GXX 4.2 or greater to build CMake on OpenBSD\n"
        "${CMAKE_CXX_COMPILER} version is: ${_GXX_VERSION}")
    endif()
  endif()
endif()

#-----------------------------------------------------------------------
# The main section of the CMakeLists file
#
#-----------------------------------------------------------------------
include(Source/CMakeVersion.cmake)

# Include the standard Dart testing module
enable_testing()
include (${CMAKE_ROOT}/Modules/Dart.cmake)

# Set up test-time configuration.
set_directory_properties(PROPERTIES
  TEST_INCLUDE_FILE "${CMake_BINARY_DIR}/Tests/EnforceConfig.cmake")

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  # where to write the resulting executables and libraries
  set(BUILD_SHARED_LIBS OFF)
  set(EXECUTABLE_OUTPUT_PATH "" CACHE INTERNAL "No configurable exe dir.")
  set(LIBRARY_OUTPUT_PATH "" CACHE INTERNAL
    "Where to put the libraries for CMake")

  # Load install destinations.
  include(Source/CMakeInstallDestinations.cmake)

  if(BUILD_TESTING)
    include(${CMake_SOURCE_DIR}/Tests/CMakeInstall.cmake)
  endif()

  # Checks for cmSystemTools.
  if(WIN32)
    set(HAVE_UNSETENV 0)
    set(HAVE_ENVIRON_NOT_REQUIRE_PROTOTYPE 1)
  else()
    include(CheckSymbolExists)
    CHECK_SYMBOL_EXISTS(unsetenv "stdlib.h" HAVE_UNSETENV)
    CHECK_SYMBOL_EXISTS(environ "stdlib.h" HAVE_ENVIRON_NOT_REQUIRE_PROTOTYPE)
  endif()
endif()

# CMAKE_TESTS_CDASH_SERVER: CDash server used by CMake/Tests.
#
# If not defined or "", this variable defaults to the server at
# "http://open.cdash.org".
#
# If set explicitly to "NOTFOUND", curl tests and ctest tests that use
# the network are skipped.
#
# If set to something starting with "http://localhost/", the CDash is
# expected to be an instance of CDash used for CDash testing, pointing
# to a cdash4simpletest database. In these cases, the CDash dashboards
# should be run first.
#
if("x${CMAKE_TESTS_CDASH_SERVER}" STREQUAL "x" AND NOT CMake_TEST_NO_NETWORK)
  set(CMAKE_TESTS_CDASH_SERVER "http://open.cdash.org")
endif()

if(CMake_TEST_EXTERNAL_CMAKE)
  set(KWIML_TEST_ENABLE 1)
  add_subdirectory(Utilities/KWIML)
endif()

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  # build the utilities (a macro defined in this file)
  CMAKE_BUILD_UTILITIES()

  if(BUILD_QtDialog)
    if(APPLE)
      set(CMAKE_BUNDLE_VERSION
        "${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}")
      set(CMAKE_BUNDLE_LOCATION "${CMAKE_INSTALL_PREFIX}")
      # make sure CMAKE_INSTALL_PREFIX ends in /
      if(NOT CMAKE_INSTALL_PREFIX MATCHES "/$")
        set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/")
      endif()
      set(CMAKE_INSTALL_PREFIX
        "${CMAKE_INSTALL_PREFIX}CMake.app/Contents")
    endif()
  endif()

  if(UNIX)
    # Install executables with the RPATH set for libraries outside the build tree.
    # This is also suitable for binaries in the build tree.  Avoid re-link on install.
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON CACHE BOOL "Install with RPATH set to find custom-built libraries.")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON CACHE BOOL "Build with RPATH set to match install-tree RPATH.")
    mark_as_advanced(CMAKE_INSTALL_RPATH_USE_LINK_PATH CMAKE_BUILD_WITH_INSTALL_RPATH)
  endif()

  # add the uninstall support
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    @ONLY)
  add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

  include (CMakeCPack.cmake)

endif()

# setup some Testing support (a macro defined in this file)
CMAKE_SETUP_TESTING()

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  if(NOT CMake_VERSION_IS_RELEASE)
    if((CMAKE_C_COMPILER_ID STREQUAL "GNU" AND
        NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS 4.2) OR
       (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND
        NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS 3.0 AND
        NOT "x${CMAKE_C_SIMULATE_ID}" STREQUAL "xMSVC") OR
       CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
      set(C_FLAGS_LIST -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts
                       -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security
                       -Wmissing-format-attribute -fno-common -Wundef
      )
      set(CXX_FLAGS_LIST -Wnon-virtual-dtor -Wcast-align -Wchar-subscripts -Wall -W
                         -Wshadow -Wpointer-arith -Wformat-security -Wundef
      )

      foreach(FLAG_LANG  C CXX)
        foreach(FLAG ${${FLAG_LANG}_FLAGS_LIST})
          if(NOT " ${CMAKE_${FLAG_LANG}_FLAGS} " MATCHES " ${FLAG} ")
            set(CMAKE_${FLAG_LANG}_FLAGS "${CMAKE_${FLAG_LANG}_FLAGS} ${FLAG}")
          endif()
        endforeach()
      endforeach()

      unset(C_FLAGS_LIST)
      unset(CXX_FLAGS_LIST)
    endif()
  endif()

  # build the remaining subdirectories
  add_subdirectory(Source)
  add_subdirectory(Utilities)
endif()

add_subdirectory(Tests)

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  if(BUILD_TESTING)
    CMAKE_SET_TARGET_FOLDER(CMakeLibTests "Tests")
    IF(TARGET CMakeServerLibTests)
      CMAKE_SET_TARGET_FOLDER(CMakeServerLibTests "Tests")
    ENDIF()
  endif()
  if(TARGET documentation)
    CMAKE_SET_TARGET_FOLDER(documentation "Documentation")
  endif()
endif()

if(BUILD_TESTING)
  add_test(SystemInformationNew "${CMAKE_CMAKE_COMMAND}"
    --system-information  -G "${CMAKE_GENERATOR}" )
endif()

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  # Install license file as it requires.
  install(FILES Copyright.txt DESTINATION ${CMAKE_DOC_DIR})

  # Install script directories.
  install(
    DIRECTORY Help Modules Templates
    DESTINATION ${CMAKE_DATA_DIR}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE
                          GROUP_READ GROUP_EXECUTE
                          WORLD_READ WORLD_EXECUTE
    PATTERN "*.sh*" PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE
                                GROUP_READ GROUP_EXECUTE
                                WORLD_READ WORLD_EXECUTE
    REGEX "/(ExportImportList|cpp)$"
      PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE
                  GROUP_READ GROUP_EXECUTE
                  WORLD_READ WORLD_EXECUTE
    REGEX "Help/(dev|guide)($|/)" EXCLUDE
    )

  # Install auxiliary files integrating with other tools.
  add_subdirectory(Auxiliary)

  # Optionally sign installed binaries.
  if(CMake_INSTALL_SIGNTOOL)
    configure_file(Source/CMakeInstallSignTool.cmake.in Source/CMakeInstallSignTool.cmake @ONLY)
    install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/Source/CMakeInstallSignTool.cmake)
  endif()
endif()
