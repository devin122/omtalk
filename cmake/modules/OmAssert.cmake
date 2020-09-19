if(OM_ASSERT_)
	return()
endif()
set(OM_ASSERT_ TRUE)

include(OmUtil)

function(om_assert)
	cmake_parse_arguments(ARG "" "MSG" "" ${ARGN})
	set(condition ${ARG_UNPARSED_ARGUMENTS})
	set_default(ARG_MSG "${condition}")
	if(${condition})
	else()
		message(FATAL_ERROR ${ARG_MSG})
	endif()
endfunction()