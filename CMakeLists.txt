cmake_minimum_required(VERSION 3.9.0)

project(KEngine2)

include_directories(ThirdParty)
include_directories("C:\\VulkanSDK\\1.1.70.1\\Include")
include_directories(Framework)
include_directories(Renderer)

add_subdirectory(Framework)
add_subdirectory(ThirdParty)
add_subdirectory(Game)
add_subdirectory(GamePlay)
add_subdirectory(Renderer)

