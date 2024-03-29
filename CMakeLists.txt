
# Default to CMAKE_BUILD_TYPE = Release unless overridden on command line
# http://www.cmake.org/pipermail/cmake/2008-September/023808.html
if( DEFINED CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Set to either \"Release\" or \"Debug\"" )
else()
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Set to either \"Release\" or \"Debug\"" )
endif()

project( kicad )

cmake_minimum_required( VERSION 2.8.4 FATAL_ERROR )
# because of http://public.kitware.com/Bug/view.php?id=10395

# Path to local CMake modules.
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules )

#
# KiCad build options should be added below.
#
# If you add a new build option, please add it's state to the CopyVersionInfoToClipboard()
# function in common/basicframe.cpp so that build option settings can be included in bug
# reports.
#

option( USE_KIWAY_DLLS
    "Build the major modules as KIFACE DLLs or DSOs, will soon be the norm." OFF )

# The desire is to migrate designs *away from* case independence, and to create designs which use
# literally (case specific) interpreted component names.  But for backwards compatibility,
# you may turn OFF this option if you really must.  (Remember that with KiCad using text
# data files, typically you would be better off simply doctoring those files into
# a case literal state with a text editor and move forward into the brave new
# world of case specificity.  Also, BOM generators may not work properly when you
# have this option turned OFF, the xml export's referential integrity is broken
# on library part name.  Hence the default is ON now, as of 29-Jan-2014.
option( KICAD_KEEPCASE
    "ON= case specific string matching on component names, OFF= match names as if they were spelt using uppercase."
    ON
    )

option( USE_WX_GRAPHICS_CONTEXT
    "Use wxGraphicsContext for rendering ( default OFF). Warning, this is experimental" )

option( USE_WX_OVERLAY
    "Use wxOverlay: Always ON for MAC ( default OFF). Warning, this is experimental" )

option( KICAD_SCRIPTING
    "set this option ON to build the scripting support inside kicad binaries"
    )

option( KICAD_SCRIPTING_MODULES
    "set this option ON to build kicad modules that can be used from scripting languages"
    )

option( KICAD_SCRIPTING_WXPYTHON
    "set this option ON to build wxpython implementation for wx interface building in python and py.shell"
    )

option( KICAD_BUILD_STATIC
    "Builds Kicad and all libraries static"
    )

option( KICAD_BUILD_DYNAMIC
    "Builds Kicad and all libraries dynamic (required for wxPython)"
    )


# WARNING: KiCad developers strongly advise you to build Boost with supplied patches,
# as it is known to work with KiCad.  Other versions may contain bugs that may result
# in KiCad errors.
option( KICAD_SKIP_BOOST
    "Skip building downloaded Boost library components, use Boost from your system."
    )
mark_as_advanced( KICAD_SKIP_BOOST )    # Normal builders should build Boost.


# when option KICAD_SCRIPTING OR KICAD_SCRIPTING_MODULES is enabled:
# PYTHON_EXECUTABLE can be defined when invoking cmake
# ( use -DPYTHON_EXECUTABLE=<python path>/python.exe or python2 )
# when not defined by user, the default is python.exe under Windows and python2 for others
# python binary file should be is exec path.

option( BUILD_GITHUB_PLUGIN "Build the GITHUB_PLUGIN for pcbnew." OFF )

# This can be set to a custom name to brag about a particular branch in the "About" dialog:
set( KICAD_REPO_NAME "product" CACHE STRING "Name of the tree from which this build came." )


# All CMake downloads go here.  Suggested is up in the source tree, not in the build dir where they
# would have to be downloaded over and over again.  The default is to choose a directory that is
# hidden on linux (starts with a '.') because there is a way to exclude this directory when grepping
# the source tree.
set( DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/.downloads-by-cmake
    CACHE PATH "Location of KiCad downloads, suggested is a dir common to all builds, i.e. global." )

#Add option to add user directories for linker, if any
LINK_DIRECTORIES( ${LINK_DIRECTORIES_PATH} )

if( UNIX )
    set( KICAD_USER_CONFIG_DIR $ENV{HOME} CACHE PATH "Location of user specific KiCad config files" )
elseif( MINGW )
    set( KICAD_USER_CONFIG_DIR $ENV{%APPDATA%} CACHE PATH "Location of user specific KiCad config files" )
endif()
mark_as_advanced( KICAD_USER_CONFIG_DIR )


#================================================
# Set flags for GCC, or treat llvm as GCC
#================================================

if( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )

    execute_process( COMMAND ${CMAKE_C_COMPILER} -dumpversion
        OUTPUT_VARIABLE GCC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE )

    # Establish -Wall early, so specialized relaxations of this may come
    # subsequently on the command line, such as in pcbnew/github/CMakeLists.txt
    set( CMAKE_C_FLAGS   "-Wall ${CMAKE_C_FLAGS}" )
    set( CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}" )

    # The optimization level is -O1 instead of the usual -O2 level because
    # boost::polygon has a function (inflate polygon) broken by the -O2 level
    # with GCC 4.7.0 to 4.7.2 (works fine with  with GCC 4.6 and 4.7.3).
    # This lower optimization level does not have a significant change on the speed.
    # See also:
    #   https://bugs.launchpad.net/kicad/+bug/1056926
    #   https://svn.boost.org/trac/boost/ticket/7983
    if( GCC_VERSION VERSION_EQUAL 4.7.0 OR ( GCC_VERSION VERSION_GREATER 4.7.0 AND GCC_VERSION VERSION_LESS 4.7.3 ) )
        set( CMAKE_C_FLAGS_RELEASE   "-O1" )
        set( CMAKE_CXX_FLAGS_RELEASE "-O1" )
    else()
        set( CMAKE_C_FLAGS_RELEASE   "-O2" )
        set( CMAKE_CXX_FLAGS_RELEASE "-O2" )
    endif()

    set( CMAKE_C_FLAGS_DEBUG   "-g3 -ggdb3 -DDEBUG" )
    set( CMAKE_CXX_FLAGS_DEBUG "-g3 -ggdb3 -DDEBUG -Wno-deprecated-declarations" )

    set( CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG" )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG" )

    if( GXX_HAS_VISIBILITY_FLAG AND NOT APPLE )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden" )
    endif()

    if( GXX_HAS_VISIBILITY_INLINES_FLAG AND NOT APPLE )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden" )
    endif()

    find_package( OpenMP QUIET )
    if( OPENMP_FOUND )
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
        add_definitions( -DUSE_OPENMP )
    endif()

    if( MINGW )
        set( CMAKE_EXE_LINKER_FLAGS_RELEASE "-s" )

        # _UNICODE definition seems needed under mingw/gcc 4.8
        # (Kicad uses unicode, and on Windows, wxWidgets >= 2.9.4 is mandatory
        # and uses unicode)
        if( GCC_VERSION VERSION_EQUAL 4.8.0 OR GCC_VERSION VERSION_GREATER 4.8.0 )
            add_definitions(-D_UNICODE)
        endif()

        # Since version 2.8.5, Cmake uses a response file (.rsp) to
        # pass the list of include paths to gcc
        # unfortunately, under mingw32+msys, at least with gcc 4.8 and previous,
        # this file is not expanded and include paths defined in this file are not taken in account
        # this is the case of wxWidgets includes
        # Disable this response file for includes ( See Windows-GNU.cmake module )
        if( WIN32 AND MSYS AND NOT CMAKE_CROSSCOMPILING )
            # fixme: it is needed only with MSYS+MINGW32? or always under MINGW
            if( ${CMAKE_SIZEOF_VOID_P} MATCHES 4 )
                set( CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 0 )
            endif()
        endif()

    else()
        # We build DLL/DSOs from static libraries, so create position independent
        # code for all cases, since we do not have DLL/DSO specific static
        # libraries. Subdirectories via add_subdirectores() reference this
        # variable, and it is either set or empty, empty for Windows.
        set( PIC_FLAG -fPIC )

        set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${PIC_FLAG}" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PIC_FLAG}" )

        if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
            set( TO_LINKER -XLinker )
        else()
            set( TO_LINKER -Wl )
        endif()

        # Thou shalt not link vaporware and tell us it's a valid DSO (apple ld doesn't support it)
        if( NOT APPLE )
            set( CMAKE_SHARED_LINKER_FLAGS "${TO_LINKER},--no-undefined" )
            set( CMAKE_MODULE_LINKER_FLAGS "${TO_LINKER},--no-undefined" )

            set( CMAKE_EXE_LINKER_FLAGS_RELEASE "-s" )
        endif()

    endif()

    # quiet GCC while in boost
    if( GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8 )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs" )
    endif()
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing" )

    if( APPLE )

        set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -D__ASSERTMACROS__ -mmacosx-version-min=10.5" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ASSERTMACROS__ -mmacosx-version-min=10.5" )

        # Allows .dylib relocation in the future - needed by fixbundle
        set( CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -headerpad_max_install_names")

        if( NOT CMAKE_CXX_COMPILER )
            EXEC_PROGRAM( wx-config ARGS --cc  OUTPUT_VARIABLE CMAKE_C_COMPILER )
        endif( NOT CMAKE_CXX_COMPILER )

        if( NOT CMAKE_CXX_COMPILER )
            EXEC_PROGRAM( wx-config ARGS --cxx OUTPUT_VARIABLE CMAKE_CXX_COMPILER )
        endif( NOT CMAKE_CXX_COMPILER )

    endif()

endif( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )

if( KICAD_KEEPCASE )
    add_definitions( -DKICAD_KEEPCASE )
endif()

if( USE_WX_OVERLAY OR APPLE )
    add_definitions( -DUSE_WX_OVERLAY )
endif()

if( KICAD_SCRIPTING )
    add_definitions( -DKICAD_SCRIPTING )
endif()

if( KICAD_SCRIPTING_MODULES )
    add_definitions( -DKICAD_SCRIPTING_MODULES )
endif()

if( KICAD_SCRIPTING_WXPYTHON )
    add_definitions( -DKICAD_SCRIPTING_WXPYTHON )
endif()

if( USE_WX_GRAPHICS_CONTEXT )
    add_definitions( -DUSE_WX_GRAPHICS_CONTEXT )
endif()



# Allow user to override the default settings for adding images to menu items.  By default
# images in menu items are enabled on all platforms except OSX.  This can be over ridden by
# defining -DUSE_IMAGES_IN_MENUS=ON/OFF to force the preferred behavior.
if( NOT DEFINED USE_IMAGES_IN_MENUS )
    if( NOT APPLE )
        set( USE_IMAGES_IN_MENUS ON )
    endif()
else()
    if( USE_IMAGES_IN_MENUS )
        set( USE_IMAGES_IN_MENUS ON )
    endif()
endif()


# KIFACE_SUFFIX is the file extension used for top level program modules which
# implement the KIFACE interface.  A valid suffix starts with a period '.'.

if( false )
    # This is the eventual situation near milestone C) of modular-kicad blueprint.
    # Until then we use .kiface extension so we don't collide with python DSO.
    set( KIFACE_SUFFIX  ${CMAKE_SHARED_MODULE_SUFFIX} )
else()
    # Temporary situation until we can dovetail the python DSO into the kiface DSO.
    set( KIFACE_SUFFIX  ".kiface" )
endif()

# KIFACE_PREFIX is a basename prefix used for top level program modules which
# implement the KIFACE.
set( KIFACE_PREFIX  "_" )
#message( STATUS "KIFACE_SUFFIX:${KIFACE_SUFFIX}  KIFACE_PREFIX:${KIFACE_PREFIX}" )


#================================================
# Locations for install targets.
set( KICAD_BIN bin
    CACHE PATH "Location of KiCad binaries." )

set( KICAD_FP_LIB_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}"
     CACHE PATH "Default path where footprint libraries are installed." )

if( UNIX )
    # Everything without leading / is relative to CMAKE_INSTALL_PREFIX.
    set( KICAD_PLUGINS lib/kicad/plugins
        CACHE PATH "Location of KiCad plugins." )
    set( KICAD_DATA share/kicad
        CACHE PATH "Location of KiCad data files." )
    set( KICAD_DOCS share/doc/kicad
        CACHE PATH "Location of KiCad documentation files." )
    set( KICAD_FP_LIB_INSTALL_PATH "${KICAD_FP_LIB_INSTALL_PATH}/share/kicad/modules" )
endif()

if( MINGW )
    # Everything without leading / is relative to CMAKE_INSTALL_PREFIX.
    set( KICAD_PLUGINS ${KICAD_BIN}/plugins
        CACHE PATH "Location of KiCad plugins." )
    set( KICAD_DATA share
        CACHE PATH "Location of KiCad data files." )
    set( KICAD_DOCS doc
        CACHE PATH "Location of KiCad documentation files." )
    set( KICAD_FP_LIB_INSTALL_PATH "${KICAD_FP_LIB_INSTALL_PATH}/modules" )
endif()

set( KICAD_DEMOS ${KICAD_DATA}/demos
    CACHE PATH "Location of KiCad demo files." )
set( KICAD_TEMPLATE ${KICAD_DATA}/template
    CACHE PATH "Location of KiCad template files." )

mark_as_advanced( KICAD_BIN
    KICAD_PLUGINS
    KICAD_DATA
    KICAD_DOCS
    KICAD_DEMOS
    KICAD_TEMPLATE )

include( Functions )

include( ExternalProject )

#================================================
# Find libraries that are needed to build KiCad.
#================================================
include( CheckFindPackageResult )

# Turn on wxWidgets compatibility mode for some classes
add_definitions(-DWX_COMPATIBILITY)

#######################
# Find OpenGL library #
#######################
find_package( OpenGL QUIET )
check_find_package_result( OPENGL_FOUND "OpenGL" )

# Handle target used to specify if a target needs wx-widgets or other libraries
# Always defined, empty if no libraries are to be build
add_custom_target( lib-dependencies )

if( KICAD_BUILD_STATIC OR KICAD_BUILD_DYNAMIC )

    # This should be build in all cases, if swig exec is not avaiable
    # will be impossible also enable SCRIPTING being for PCBNEW required immediatly

    include( download_pcre )
    include( download_swig )


    #set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so;.dylib;.dll")

    if( KICAD_BUILD_STATIC AND KICAD_BUILD_DYNAMIC )
        message(FATAL_ERROR "You can't build STATIC and DYNAMIC together")
    endif()

    if( KICAD_BUILD_STATIC )
        message(STATUS "KICAD_BUILD_STATIC set")
        if( KICAD_SCRIPTING OR KICAD_SCRIPTING_WXPYTHON OR KICAD_SCRIPTING_MODULES )
            message(FATAL_ERROR "KICAD_SCRIPTING* is not supported with KICAD_BUILD_STATIC, please select KICAD_BUILD_DYNAMIC" )
        endif()
    endif()

    if( KICAD_BUILD_DYNAMIC )
        message(STATUS "KICAD_BUILD_DYNAMIC set")
    endif()


    if( KICAD_SCRIPTING OR KICAD_SCRIPTING_WXPYTHON OR KICAD_SCRIPTING_MODULES )
 
        message(STATUS "Scripting ENABLED")        
        include( download_wxpython )

        set( SWIG_EXECUTABLE  ${SWIG_ROOT}/bin/swig )
        set( SWIG_INCLUDE     ${SWIG_ROOT}/include )
        set( PYTHON_DEST ${LIBWXPYTHON_ROOT}/wxPython/lib/python2.6/site-packages )

        if( NOT EXISTS ${SWIG_EXECUTABLE} )
            set(KICAD_SCRIPTING CACHE OFF FORCE "Disabling KICAD_SCRIPTING")
            message( STATUS  "KICAD_SCRIPTING Enabled but SWIG not found, please disable and before reenabling execute: make swig")
            message( FATAL_ERROR "Missing SWIG!")
        endif()
        message(STATUS "SWIG_EXECUTABLE: ${SWIG_EXECUTABLE}")

        set( PYTHON_DEST ${LIBWXPYTHON_ROOT}/wxPython/lib/python2.6/site-packages )

        set(wxWidgets_BIN_DIR           ${LIBWXPYTHON_ROOT}/bin/wxrc )
        set(wxWidgets_CONFIG_EXECUTABLE ${LIBWXPYTHON_ROOT}/bin/wx-config )
        set(wxWidgets_INCLUDE_DIRS      ${LIBWXPYTHON_ROOT}/include/wx-3.0 )
        set(wxWidgets_LIBRARY_DIRS      ${LIBWXPYTHON_ROOT}/lib )

        add_dependencies( lib-dependencies libwxpython )
        add_dependencies( lib-dependencies swig )

    else()
        include( download_wxwidgets )
        add_dependencies( lib-dependencies libwx )
    endif()

    include( download_libpng )

    include( download_pkgconfig )
        set( PKG_CONFIG_EXECUTABLE "${PKGCONFIG_ROOT}/bin/pkg-config" )
    include( download_glew )
        set( GLEW_GLEW_LIBRARY "${GLEW_ROOT}/lib/libGLEW.a" )
        set( GLEW_INCLUDE_DIR  "${GLEW_ROOT}/include" )
    include( download_pixman )
        set( PIXMAN_LIBRARY     "${PIXMAN_ROOT}/lib/libpixman-1.a" )
    include( download_cairo )
        set( CAIRO_INCLUDE_DIR  "${CAIRO_ROOT}/include/cairo" )
        set( CAIRO_LIBRARY      "${CAIRO_ROOT}/lib/libcairo.a" )

    add_dependencies( lib-dependencies boost )
    add_dependencies( lib-dependencies cairo )
    add_dependencies( lib-dependencies libpng )
    add_dependencies( lib-dependencies pixman )
    add_dependencies( lib-dependencies pkgconfig )

    if( KICAD_BUILD_DYNAMIC AND APPLE )
        add_custom_target( osx_fix_bundles ALL DEPENDS cvpcb eeschema gerbview kicad pcbnew bitmap2component pcb_calculator pl_editor)
        add_custom_command(TARGET osx_fix_bundles POST_BUILD COMMAND scripts/osx_fixbundle.sh COMMENT "Migrating dylibs to bundles")
    endif()

endif( KICAD_BUILD_STATIC OR KICAD_BUILD_DYNAMIC)

#####################
# Find GLEW library #
#####################
find_package(GLEW)
check_find_package_result(GLEW_FOUND "GLEW")

######################
# Find Cairo library #
######################
find_package(Cairo 1.8.1 QUIET)
check_find_package_result(CAIRO_FOUND "Cairo")

# Download boost and possibly build parts of it
#################################################
if( KICAD_SKIP_BOOST )
    find_package( Boost 1.54.0 REQUIRED COMPONENTS context date_time filesystem iostreams locale
                                                   program_options regex system thread )

    if( NOT Boost_FOUND )
        message( FATAL_ERROR "Boost 1.54+ libraries are required." )
    endif()

    message( WARNING "
WARNING: You decided to skip building boost library.
KiCad developers strongly advise you to build the bundled boost library, as it is known to work with KiCad.
Other versions may contain bugs that may result in KiCad errors." )
else()
    include( download_boost )
endif()

##########################
# Find wxWidgets library #
##########################
# Here you can define what libraries of wxWidgets you need for your
# application. You can figure out what libraries you need here;
# http://www.wxwidgets.org/manuals/2.8/wx_librarieslist.html

# On Apple only wxwidgets 2.9 or higher doesn't need to find aui part of base
# Seems no more needed on wx-3
if( APPLE AND ( KICAD_SCRIPTING OR KICAD_SCRIPTING_MODULES OR KICAD_SCRIPTING_WXPYTHON) )
    find_package( wxWidgets COMPONENTS gl adv html core net base xml QUIET )
else()
    find_package( wxWidgets COMPONENTS gl aui adv html core net base xml QUIET )
endif()

if( NOT (KICAD_BUILD_STATIC OR KICAD_BUILD_DYNAMIC) )
check_find_package_result( wxWidgets_FOUND "wxWidgets" )
endif( NOT (KICAD_BUILD_STATIC OR KICAD_BUILD_DYNAMIC) )

# Include wxWidgets macros.
include( ${wxWidgets_USE_FILE} )

# Include MinGW resource compiler.
include( MinGWResourceCompiler )

# Generate build system specific header file.
include( PerformFeatureChecks )
perform_feature_checks()

# Find GDI+ on windows if wxGraphicsContext is available.
if( MINGW AND USE_WX_GRAPHICS_CONTEXT )
    find_package( GdiPlus )
    check_find_package_result( GDI_PLUS_FOUND "GDI+" )
endif()

# Assist with header file searching optimization:
# INC_BEFORE and INC_AFTER are two lists which go at the front and back of the
# header file search lists, respectively.
# INC_BEFORE is for use with "include_directories( BEFORE ...)", which _reverses_
# the order during insertion.  (So put first wanted last, which is
# ${CMAKE_SOURCE_DIR/include.)  Use '.' for current source dir since
# we don't want expansion here and now, which would happen if using ${CMAKE_CURRENT_SOURCE_DIR}.
# Instead we use '.' which is applicable to any source directory below here as a result of
# this lack of expansion.
set( INC_BEFORE
    .
    ${CMAKE_SOURCE_DIR}/include
    )

set( INC_AFTER
    ${Boost_INCLUDE_DIR}
    #include <config.h>
    ${CMAKE_BINARY_DIR}
    )


# Find Python and other scripting resources
if( KICAD_SCRIPTING OR KICAD_SCRIPTING_MODULES )

    if( APPLE )
        set( PYTHON_LIBRARY /System/Library/Frameworks/Python.framework/Versions/2.6/Python  )
        set( PYTHON_INCLUDE_DIR /System/Library/Frameworks/Python.framework/Versions//2.6/include/python2.6 )
        set( PythonInterp_FIND_VERSION 2.6 )
        set( PythonLibs_FIND_VERSION 2.6 )
    endif()
    
    # force a python version < 3.0
    set( PythonInterp_FIND_VERSION 2.6)
    set( PythonLibs_FIND_VERSION 2.6 )

    find_package( PythonInterp )
    
    check_find_package_result( PYTHONINTERP_FOUND "Python Interpreter" )
    
    if( NOT PYTHON_VERSION_MAJOR EQUAL 2 )
        message( FATAL_ERROR "Python 2.x is required." )
    endif()
    # Get the correct Python site package install path from the Python interpreter found by
    # FindPythonInterp unless the user specifically defined a custom path.
    if( NOT PYTHON_SITE_PACKAGE_PATH )
        execute_process( COMMAND ${PYTHON_EXECUTABLE} -c "import distutils.sysconfig;print\"%s\"%distutils.sysconfig.get_python_lib()"
            OUTPUT_VARIABLE PYTHON_SITE_PACKAGE_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        if( NOT PYTHON_SITE_PACKAGE_PATH )
            message( FATAL_ERROR "Error occurred while attemping to find the Python site library path." )
        endif()
    endif()

    set( PYTHON_DEST "${PYTHON_SITE_PACKAGE_PATH}" CACHE PATH "Python module install path." )
    mark_as_advanced( PYTHON_DEST )
    message( STATUS "Python module install path: ${PYTHON_DEST}" )

    find_package( PythonLibs 2.6 )

    #message( STATUS "PYTHON_INCLUDE_DIRS:${PYTHON_INCLUDE_DIRS}" )

    # Infrequently needed headers go at end of search paths, append to INC_AFTER which
    # although is used for all components, should be a harmless hit for something like eeschema
    # so long as unused search paths are at the end like this.
    set( INC_AFTER ${INC_AFTER} ${PYTHON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/scripting )

    #message( STATUS "/ INC_AFTER:${INC_AFTER}" )

endif()


# Automagically create version header file.
include( CreateBzrVersionHeader )
create_bzr_version_header()

if( EXISTS ${CMAKE_SOURCE_DIR}/include/config.h )
    # This file may exist ( created by an alternate process to the svn test above),
    # e.g. could be created by a checkout script that produces a source tar file.
    # This directive means the about box will have the svn date & revision in it,
    # but the hard coded release date ( program version) will be preserved.
    add_definitions( -DHAVE_SVN_REVISION )
endif()


#================================================
# Let CMake look in these directories for nested
# 'CMakeLists.txt' files to process
#================================================

############################
# Binaries ( CMake targets ) #
############################

add_subdirectory( bitmaps_png )
add_subdirectory( common )
add_subdirectory( 3d-viewer )
add_subdirectory( cvpcb )
add_subdirectory( eeschema )
add_subdirectory( gerbview )
add_subdirectory( kicad )
add_subdirectory( lib_dxf )
add_subdirectory( pcbnew )
add_subdirectory( polygon )
add_subdirectory( pagelayout_editor )
add_subdirectory( potrace )
add_subdirectory( bitmap2component )
add_subdirectory( pcb_calculator )
add_subdirectory( tools )
add_subdirectory( utils )
add_subdirectory( qa )
#add_subdirectory( new )


# Make all libs and executables depend on ExternalProject_Add( boost ),
# except perhaps bitmap lib.  This allows a multi-threaded build to succeed.
add_dependencies( pcbnew boost )
add_dependencies( eeschema boost )
add_dependencies( cvpcb boost )
add_dependencies( gal boost )
add_dependencies( common boost )
add_dependencies( pcbcommon boost )
add_dependencies( 3d-viewer boost )
add_dependencies( pcad2kicadpcb boost )
add_dependencies( polygon boost )
add_dependencies( pl_editor boost )
add_dependencies( pnsrouter boost )


if ( KICAD_BUILD_STATIC OR KICAD_BUILD_DYNAMIC )
add_dependencies( pcbnew lib-dependencies )
add_dependencies( eeschema lib-dependencies )
add_dependencies( cvpcb lib-dependencies )
add_dependencies( common lib-dependencies )
add_dependencies( gal lib-dependencies )
add_dependencies( pcbcommon lib-dependencies )
add_dependencies( 3d-viewer lib-dependencies )
add_dependencies( pcad2kicadpcb lib-dependencies )
add_dependencies( pl_editor lib-dependencies )
add_dependencies( pnsrouter lib-dependencies )
endif()

if ( KICAD_BUILD_DYNAMIC )
endif()

#############
# Resources #
#############
add_subdirectory( demos )
add_subdirectory( template )

#================================================
# Doxygen Output
#================================================
find_package( Doxygen )
if( DOXYGEN_FOUND )
    add_custom_target( doxygen-docs
        ${CMAKE_COMMAND} -E remove_directory Documentation/doxygen
        COMMAND ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS Doxyfile
        COMMENT "building doxygen docs into directory Documentation/doxygen/html"
        )
else()
    message( STATUS "WARNING: Doxygen not found - doxygen-docs (Source Docs) target not created" )
endif()


#================================================
# "make uninstall" rules
#================================================
configure_file(
    "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY )

add_custom_target( uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" )


#================================================
# Installation
#================================================

install( FILES INSTALL.txt
    DESTINATION ${KICAD_DOCS}
    COMPONENT resources )

install( FILES resources/freeroute.jnlp
    DESTINATION ${KICAD_BIN}
    COMPONENT resources )

###
# Install scripts
###
if( UNIX )
    install( DIRECTORY scripts
        DESTINATION ${KICAD_DOCS}
        COMPONENT resources
        )
endif()

###
# FreeDesktop .desktop and MIME resources
###
if( UNIX )

    # Set paths
    set( UNIX_MIME_DIR resources/linux/mime )
    set( UNIX_MIMELNK_FILES ${UNIX_MIME_DIR}/mimelnk )
    set( UNIX_MIME_FILES ${UNIX_MIME_DIR}/mime )
    set( UNIX_ICON_FILES ${UNIX_MIME_DIR}/icons )
    set( UNIX_APPLICATIONS_FILES ${UNIX_MIME_DIR}/applications )

    # Install Mimelnk directory
    install( DIRECTORY ${UNIX_MIMELNK_FILES}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share
        COMPONENT resources
        )

    # Install Mime directory
    install( DIRECTORY ${UNIX_ICON_FILES}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share
        COMPONENT resources
        )

    # Install Icons
    install( DIRECTORY ${UNIX_MIME_FILES}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share
        COMPONENT resources
        )

    # Install Applications directory (.desktop files)
    install( DIRECTORY ${UNIX_APPLICATIONS_FILES}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share
        COMPONENT resources
        )
endif()

#include( CTest )


if( UNIX AND NOT APPLE )

    # Create a *.deb file:
    set( CPACK_GENERATOR "DEB" )
    set( CPACK_DEBIAN_PACKAGE_MAINTAINER "http://launchpad.net/kicad" )

    set( CPACK_PACKAGE_VERSION_MAJOR 1 )
    set( CPACK_PACKAGE_VERSION_MINOR 0 )
    set( CPACK_PACKAGE_VERSION_PATCH 0 )
    #set( CPACK_PACKAGE_CONTACT Firstname Lastname <email@company.com> )
    set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "KiCad built by CMake build system." )

    include( CPack )

endif()
