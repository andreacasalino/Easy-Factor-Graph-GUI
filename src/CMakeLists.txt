SET(BUILD_EFG_SAMPLES OFF CACHE BOOL "Override EFG sample option" FORCE)
SET(THREAD_POOL_OPT OFF CACHE BOOL "Override THREAD_POOL_OPT" FORCE)
include(FetchContent)
FetchContent_Declare(
  efg_lib
  GIT_REPOSITORY https://github.com/andreacasalino/Easy-Factor-Graph.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(efg_lib)

include(FetchContent)
FetchContent_Declare(
  http_lib
  GIT_REPOSITORY https://github.com/andreacasalino/Http-GUI.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(http_lib)


set(PROJECT_SHORTNAME "EFG-GUI")

CollectSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)
GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_SHORTNAME} ${SOURCES})

target_include_directories(${PROJECT_SHORTNAME} 
PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/header
)

target_link_libraries(${PROJECT_SHORTNAME} 
PUBLIC
Http-GUI
EFG
)

target_compile_definitions(${PROJECT_SHORTNAME}
PUBLIC
	-D SOURCE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/"
)

find_package(Threads)
target_link_libraries(${PROJECT_SHORTNAME}
    PRIVATE
	${CMAKE_THREAD_LIBS_INIT}      
)


install(TARGETS ${PROJECT_SHORTNAME})
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/JS/ DESTINATION bin/JS)
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/image/ DESTINATION bin/image)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/EFG-GUI.html DESTINATION bin/)
