# creates custom commands to convert GLSL shaders to spir-v
# and creates depsList containing name of files that should be included among the source files
macro(add_shaders nameList depsList)
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders")
	foreach(name ${nameList})
		get_filename_component(directory ${name} DIRECTORY)
		if(directory)
			file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders/${directory}")
		endif()
		add_custom_command(COMMENT "Converting ${name} to spir-v..."
		                   MAIN_DEPENDENCY shaders/${name}
		                   OUTPUT shaders/${name}.spv
		                   COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V -x ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${name} -o shaders/${name}.spv)
		source_group("Shaders" FILES shaders/${name})
		source_group("Shaders/SPIR-V" FILES ${CMAKE_CURRENT_BINARY_DIR}/shaders/${name}.spv)
		list(APPEND ${depsList} shaders/${name} ${CMAKE_CURRENT_BINARY_DIR}/shaders/${name}.spv)
	endforeach()
endmacro()
