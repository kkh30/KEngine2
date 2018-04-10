
file(GLOB dot_cpp *.cpp)
file(GLOB dot_h *.h)

file(GLOB vulkan_rhi "KEVulkanRHI/*.h"  "KEVulkanRHI/*.cpp")
file(GLOB opengl_rhi "KEOpenGLRHI/*.h"  "KEOpenGLRHI/*.cpp")

source_group("VulkanRHI" FILES ${vulkan_rhi})
source_group("OpenGLRHI" FILES ${opengl_rhi})


add_library(Renderer STATIC ${dot_cpp} ${dot_h} ${vulkan_rhi} ${opengl_rhi})
target_link_libraries(Renderer Framework)



