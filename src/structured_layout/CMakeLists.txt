add_library(structured_layout INTERFACE)
add_library(structured_layout::structured_layout ALIAS structured_layout)

target_compile_features(structured_layout INTERFACE cxx_std_23)
target_include_directories(structured_layout INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(structured_layout INTERFACE urd::urd)
