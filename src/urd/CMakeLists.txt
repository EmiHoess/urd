add_library(urd INTERFACE)
add_library(urd::urd ALIAS urd)

target_compile_features(urd INTERFACE cxx_std_23)
target_include_directories(urd INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
