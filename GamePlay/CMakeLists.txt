
file(GLOB ECS "ECS/*.h" "ECS/*.cpp")
file(GLOB Camera "Camera/*.h" "Camera/*.cpp")

source_group("ECS" FILES ${ECS})
source_group("Camera" FILES ${Camera})

add_library(GamePlay STATIC ${ECS} ${Camera})
target_link_libraries(GamePlay Framework)
