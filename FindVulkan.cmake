#
# Module for finding Vulkan
#
# Targets:
#   Vulkan::Vulkan
#   Vulkan::Headers
#   Vulkan::glslangValidator
#
# Variables:
#   Vulkan_FOUND
#   Vulkan_INCLUDE_DIRS
#   Vulkan_LIBRARIES
#
# Cache variables:
#   Vulkan_INCLUDE_DIR
#   Vulkan_LIBRARY
#   Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
#


# find Vulkan include path
find_path(${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR vulkan/vulkan.h
	"$ENV{VULKAN_SDK}/Include"
	/usr/include
	/usr/local/include
)

# find Vulkan library
find_library(${CMAKE_FIND_PACKAGE_NAME}_LIBRARY
	NAMES
		vulkan vulkan-1
	PATHS
		"$ENV{VULKAN_SDK}/Lib"
		"$ENV{VULKAN_SDK}/Lib32"
		/usr/lib64
		/usr/local/lib64
		/usr/lib
		/usr/lib/x86_64-linux-gnu
		/usr/local/lib
)

# find glslangValidator
find_program(${CMAKE_FIND_PACKAGE_NAME}_GLSLANG_VALIDATOR_EXECUTABLE
	NAMES
		glslangValidator
	PATHS
		"$ENV{VULKAN_SDK}/bin"
		"$ENV{VULKAN_SDK}/bin32"
		/usr/bin
		/usr/local/bin
)


# set Vulkan_LIBRARIES and Vulkan_INCLUDE_DIRS
set(Vulkan_LIBRARIES ${Vulkan_LIBRARY})
set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIR})

# report missing variables
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan DEFAULT_MSG Vulkan_LIBRARY Vulkan_INCLUDE_DIR)


# Vulkan::Vulkan target
if(${CMAKE_FIND_PACKAGE_NAME}_FOUND)
	if(NOT TARGET ${CMAKE_FIND_PACKAGE_NAME}::${CMAKE_FIND_PACKAGE_NAME})
		add_library(${CMAKE_FIND_PACKAGE_NAME}::${CMAKE_FIND_PACKAGE_NAME} INTERFACE IMPORTED)
		set_target_properties(${CMAKE_FIND_PACKAGE_NAME}::${CMAKE_FIND_PACKAGE_NAME} PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR}"
			INTERFACE_LINK_LIBRARIES "${${CMAKE_FIND_PACKAGE_NAME}_LIBRARY}"
		)
	endif()
endif()

# Vulkan::Headers target
if(Vulkan_INCLUDE_DIR)
	if(NOT TARGET Vulkan::Headers)
		add_library(Vulkan::Headers INTERFACE IMPORTED)
		set_target_properties(Vulkan::Headers PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIR}"
		)
	endif()
endif()

# Vulkan::glslangValidator target
if(Vulkan_FOUND AND Vulkan_GLSLANG_VALIDATOR_EXECUTABLE AND NOT TARGET Vulkan::glslangValidator)
	add_executable(Vulkan::glslangValidator IMPORTED)
	set_property(TARGET Vulkan::glslangValidator PROPERTY IMPORTED_LOCATION "${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}")
endif()
