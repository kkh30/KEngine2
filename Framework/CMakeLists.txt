file(GLOB CPP_FILES *.cpp )
file(GLOB H_FILES *.h )
file(GLOB MATH_CPP_FILES "Math/*.cpp")
file(GLOB MATH_H_FILES "Math/*.h")
source_group("Math" FILES ${MATH_CPP_FILES} ${MATH_H_FILES})
include_directories("./")
add_library(Framework ${CPP_FILES} ${H_FILES} ${MATH_CPP_FILES} ${MATH_H_FILES})
target_link_libraries(Framework WSIWindow)