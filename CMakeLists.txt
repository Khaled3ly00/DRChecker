# CMakeList.txt : CMake project for DRChecker, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("DRChecker")

add_library(drchecker
    src/geometry/Vector.cpp
	src/geometry/BoundingBox.cpp
	src/geometry/Segment.cpp
    src/geometry/Point.cpp)

target_include_directories(drchecker PUBLIC 
	${PROJECT_SOURCE_DIR}/include
)

# Add source to this project's executable.
#add_executable (drchecker_tests  )

#target_link_libraries(drchecker_tests
#    PUBLIC
#       ${PROJECT_SOURCE_DIR}/include
#)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET drchecker PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
