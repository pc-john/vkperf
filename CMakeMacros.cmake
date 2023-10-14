# creates custom commands to convert GLSL shaders to spir-v
# and creates depsList containing name of files that should be included among the source files
macro(add_shaders nameList depsList)
	foreach(name ${nameList})
		get_filename_component(directory ${name} DIRECTORY)
		if(directory)
			file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${directory}")
		endif()
		add_custom_command(COMMENT "Converting ${name} to spir-v..."
		                   MAIN_DEPENDENCY ${name}
		                   OUTPUT ${name}.spv
		                   COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V -x ${CMAKE_CURRENT_SOURCE_DIR}/${name} -o ${name}.spv)
		source_group("Shaders" FILES ${name})
		source_group("Shaders/SPIR-V" FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}.spv)
		list(APPEND ${depsList} ${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}.spv)
	endforeach()
endmacro()
