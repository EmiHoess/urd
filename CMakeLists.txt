cmake_minimum_required(VERSION 3.24)

project(urd VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

option(urd_install "Install the library packages" ${PROJECT_IS_TOP_LEVEL})
option(structured_layout_build_experimental "Enable legacy union views" OFF)
option(structured_layout_build_property_views "Enable native property views" OFF)
option(structured_layout_build_reflected_views "Enable C++26 reflected views" OFF)
option(structured_layout_build_generated_union_views "Enable generated union views" OFF)

add_subdirectory(src/urd)
add_subdirectory(src/structured_layout)

set(structured_layout_export_targets structured_layout)
set(structured_layout_components "")
include(cmake/components.cmake)

if(urd_install)
    include(cmake/install.cmake)
endif()
