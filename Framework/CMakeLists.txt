file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)


add_library(Framework ${CPP_FILES} ${H_FILES} )
target_link_libraries(Framework WSIWindow)