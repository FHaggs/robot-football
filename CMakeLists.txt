cmake_minimum_required(VERSION 3.0)

project(robotfootball)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} include)
set(LIBRARIES ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

file(GLOB SOURCE_FILES main.c opengl.c ${CMAKE_CURRENT_SOURCE_DIR}/SOIL/*.c ${CMAKE_CURRENT_SOURCE_DIR}/chipmunk/*.c )
file(GLOB INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/include/SOIL*.h ${CMAKE_CURRENT_SOURCE_DIR}/include/chipmunk/*.h)

include_directories("chipmunk" "include" "include/SOIL" "include/chipmunk")
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

if(APPLE)
    add_definitions( -Wno-deprecated-declarations -O3)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${INCLUDE_FILES})

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} -lm)

