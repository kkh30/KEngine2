
#=========================VULKAN_WRAPPER==========================
option(USE_VULKAN_WRAPPER "Build a dispatch-table, to skip loader trampoline code." ON)
if (USE_VULKAN_WRAPPER)
    add_definitions(-DUSE_VULKAN_WRAPPER)
endif()
#=================================================================

add_subdirectory(WSIWindow)

