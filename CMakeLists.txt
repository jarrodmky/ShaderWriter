cmake_minimum_required( VERSION 3.15 )
cmake_policy( VERSION 3.15 )

if ( VCPKG_TARGET_TRIPLET )
	set( VCPKG_TOOLCHAIN ON )
endif ()

# Set project name, used in folders and in workspace creation
set( MAIN_PROJECT_NAME "ShaderWriter" )

# Set main project version numbers
set( VERSION_MAJOR 1 )
set( VERSION_MINOR 2 )
set( VERSION_BUILD 0 )
set( VERSION_YEAR 2021 )

if ( NOT DEFINED SDW_BUILD_SDW )
	set( SDW_BUILD_SDW ON )
endif ()

# Used to look for external modules
if ( VCPKG_TOOLCHAIN )
	set( CMAKE_MODULE_PATH
		${CMAKE_CURRENT_SOURCE_DIR}/CMake
		${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules
		${CMAKE_CURRENT_SOURCE_DIR}/CMake/Toolchains
	)
	set( CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL Release )
	set( CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release )
else ()
	set( CMAKE_MODULE_PATH
		${CMAKE_CURRENT_SOURCE_DIR}/CMake
		${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules
		${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/Traditional
		${CMAKE_CURRENT_SOURCE_DIR}/CMake/Toolchains
	)
endif ()

set( CMAKE_TEMPLATES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Templates )
set( CMAKE_POSITION_INDEPENDENT_CODE ON )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

include( PrecompiledHeaders )

set( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "The configuration types" FORCE )

# Declare the project
project( ${MAIN_PROJECT_NAME} )

include( Setup )
include( Project )
include( CompilerVersion )
include( Doxygen )
include( UnitTest )
include( CompilationFlags )
include( AStyleUtils )
include( ExternalDependencies )

# Organize projects into folders
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

if ( WIN32 )
	set( BinLibraries
		Dbghelp
	)
else ()
	set( BinLibraries
		dl
	)
endif ()

#--------------------------------------------------------------------------------------------------
#	Looking for needed libraries
#--------------------------------------------------------------------------------------------------
if ( CMAKE_COMPILER_IS_GNUCXX )
	DumpCompilerVersion( ${PROJECT_NAME}_COMPILER_VERSION )
	msg_debug( "GCC version : ${${PROJECT_NAME}_COMPILER_VERSION}" )
	if ( ${${PROJECT_NAME}_COMPILER_VERSION} LESS "49" )
		message( FATAL_ERROR "Your compiler is too old (${${PROJECT_NAME}_COMPILER_VERSION}), consider an update." )
	endif ()
elseif ( ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" )
	DumpCompilerVersion( ${PROJECT_NAME}_COMPILER_VERSION )
	msg_debug( "clang version : ${${PROJECT_NAME}_COMPILER_VERSION}" )
elseif ( MSVC )
	if ( MSVC60 OR MSVC70 OR MSVC71 OR MSVC80 OR MSVC10 OR MSVC11 OR MSVC12 )
		message( FATAL_ERROR "Your compiler is too old, consider an update." )
	endif ()
else ()
	message( FATAL_ERROR "Unsupported compiler." )
endif ()

#--------------------------------------------------------------------------------------------------
#	MSVC specifics
#--------------------------------------------------------------------------------------------------
if( MSVC )
	string( REGEX REPLACE "/Z[iI7]" "/ZI" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" )
	string( REGEX REPLACE "/W[0-4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" )
	string( REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
	string( REGEX REPLACE "/Zm[0-9]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm300" )
	add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

add_compile_definitions( MAIN_VERSION_MAJOR=${VERSION_MAJOR} )
add_compile_definitions( MAIN_VERSION_MINOR=${VERSION_MINOR} )
add_compile_definitions( MAIN_VERSION_BUILD=${VERSION_BUILD} )
add_compile_definitions( MAIN_VERSION_YEAR=${VERSION_YEAR} )

# Small macro to add subdirectory files to current target source and header files
macro( parse_subdir_files _BASE_DIR _FOLDER _GROUP )
	file( GLOB _FOLDER_HEADER_FILES
		${_BASE_DIR}/include/${PROJECT_NAME}/${_FOLDER}/*.h
		${_BASE_DIR}/include/${PROJECT_NAME}/${_FOLDER}/*.hpp
		${_BASE_DIR}/include/${PROJECT_NAME}/${_FOLDER}/*.inl
	)
	file( GLOB _FOLDER_ENUM_FILES
		${_BASE_DIR}/include/${PROJECT_NAME}/${_FOLDER}/*.enum
	)
	file( GLOB _FOLDER_SCRIPT_FILES
		${_BASE_DIR}/scripts/${PROJECT_NAME}/${_FOLDER}/*.py
	)
	file( GLOB _FOLDER_SOURCE_FILES
		${_BASE_DIR}/source/${PROJECT_NAME}/${_FOLDER}/*.h
		${_BASE_DIR}/source/${PROJECT_NAME}/${_FOLDER}/*.hpp
		${_BASE_DIR}/source/${PROJECT_NAME}/${_FOLDER}/*.inl
		${_BASE_DIR}/source/${PROJECT_NAME}/${_FOLDER}/*.c
		${_BASE_DIR}/source/${PROJECT_NAME}/${_FOLDER}/*.cpp
	)
	source_group( "Script Files\\${_GROUP}" FILES ${_FOLDER_SCRIPT_FILES} )
	source_group( "Enum Files\\${_GROUP}" FILES ${_FOLDER_ENUM_FILES} )
	source_group( "Header Files\\${_GROUP}" FILES ${_FOLDER_HEADER_FILES} )
	source_group( "Source Files\\${_GROUP}" FILES ${_FOLDER_SOURCE_FILES} )
endmacro()

#--------------------------------------------------------------------------------------------------
#	Adding include dirs to include search path
#--------------------------------------------------------------------------------------------------
if ( NOT DEFINED SDW_BUILD_TESTS )
	option( SDW_BUILD_TESTS "Build ShaderWriter test applications" OFF )
endif ()
if ( NOT DEFINED SDW_BUILD_EXPORTERS )
	option( SDW_BUILD_EXPORTERS "Build exporters" ON )
endif ()

set( CMAKE_DEBUG_POSTFIX "d" )

if ( MSVC OR NOT "${CMAKE_BUILD_TYPE}" STREQUAL "" )
	set( SDW_EXPORTERS_LIST )

	if ( NOT DEFINED SDW_BASE_DIR )
		set( SDW_BASE_DIR "ShaderWriter" )
	endif ()

	add_subdirectory( source )

	if ( SDW_BUILD_TESTS AND SDW_BUILD_EXPORTERS )
		add_subdirectory( test )
	endif ()
else()
	message( SEND_ERROR "Please select a build type (Debug, Release or RelWithDebInfo)" )
endif()
