# Opt-in common RHI probe. It shares the normal build tree and never replaces
# the validated game renderer while the common GPU path is under integration.
set(TSFP_VULKAN_SDK "" CACHE PATH "Pinned local macOS Vulkan SDK (macOS directory)")
if(TSFP_VULKAN_SDK)
    if(NOT TSFP_RUNTIME_DIR)
        get_filename_component(TSFP_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
    endif()
    set(TSFP_VK_SOURCE "${TSFP_RUNTIME_DIR}/src/d3d")
    set(TSFP_VK_THIRD "${TSFP_RUNTIME_DIR}/third_party")
    foreach(required Vulkan-Headers/include/vulkan/vulkan.h volk/volk.c VulkanMemoryAllocator/include/vk_mem_alloc.h)
        if(NOT EXISTS "${TSFP_VK_THIRD}/${required}")
            message(FATAL_ERROR "Initialize pinned Vulkan submodules: ${required}")
        endif()
    endforeach()
    add_executable(tsfp_rhi_vulkan_test
        "${TSFP_NATIVE_DIR}/../../tests/rhi_menu_sampling.c"
        "${TSFP_VK_SOURCE}/rhi.c" "${TSFP_VK_SOURCE}/rhi_vulkan.c"
        "${TSFP_VK_SOURCE}/rhi_vulkan_dxc.cpp" "${TSFP_VK_SOURCE}/rhi_vulkan_vma.cpp"
        "${TSFP_VK_SOURCE}/rhi_shader_cache.c" "${TSFP_VK_SOURCE}/d3d8_compile.c"
        "${TSFP_VK_THIRD}/volk/volk.c")
    target_include_directories(tsfp_rhi_vulkan_test PRIVATE
        "${TSFP_VK_SOURCE}" "${TSFP_RUNTIME_DIR}/src/config"
        "${TSFP_NATIVE_DIR}/.." "${TSFP_NATIVE_DIR}/../../tests"
        "${TSFP_VK_THIRD}/Vulkan-Headers/include" "${TSFP_VK_THIRD}/volk"
        "${TSFP_VK_THIRD}/VulkanMemoryAllocator/include" "${TSFP_VULKAN_SDK}/include/dxc")
    target_compile_definitions(tsfp_rhi_vulkan_test PRIVATE RHI_HAVE_VULKAN
        RHI_VULKAN_SDL VK_NO_PROTOTYPES
        "RHI_DXC_LIBRARY=\"${TSFP_VULKAN_SDK}/lib/libdxcompiler.dylib\"")
    target_link_libraries(tsfp_rhi_vulkan_test PRIVATE platform SDL3::SDL3)
    target_compile_options(tsfp_rhi_vulkan_test PRIVATE -O2)
    set_source_files_properties("${TSFP_VK_SOURCE}/rhi_vulkan_vma.cpp" PROPERTIES
        COMPILE_OPTIONS "-Wno-nullability-completeness")
    set_target_properties(tsfp_rhi_vulkan_test PROPERTIES CXX_STANDARD 17)
endif()
