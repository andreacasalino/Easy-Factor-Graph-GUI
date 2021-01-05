add_subdirectory(Http-GUI/Lib)
# set(THREAD_POOL_OPT ON)
add_subdirectory(EFG/Lib)

add_executable(EFG-GUI 
EFG-GUI.cpp 
src/EFG-model.h
src/EFG-model.cpp
src/VariableFinder.cpp 
)

target_include_directories(EFG-GUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(EFG-GUI PUBLIC
gui::Http-GUI
EFG::Easy-Factor-Graph
)

if(WIN32)
	add_custom_command(TARGET EFG-GUI POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:EFG-GUI> ${CMAKE_SOURCE_DIR}/App/EFG-GUI.exe
	)
else()
	add_custom_command(TARGET EFG-GUI POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:EFG-GUI> ${CMAKE_SOURCE_DIR}/App/EFG-GUI
	)
endif()

