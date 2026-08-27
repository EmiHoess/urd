function(urd_add_component component directory dependency standard)
    set(component_target "structured_layout_${component}")
    set(component_include "${CMAKE_INSTALL_INCLUDEDIR}/structured_layout/${component}")
    add_library(${component_target} INTERFACE)
    add_library(structured_layout::${component} ALIAS ${component_target})
    set_target_properties(${component_target} PROPERTIES EXPORT_NAME ${component})
    target_compile_features(${component_target} INTERFACE cxx_std_${standard})
    target_include_directories(${component_target} INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${directory}/include>"
        "$<INSTALL_INTERFACE:${component_include}>")
    target_link_libraries(${component_target} INTERFACE ${dependency})
    list(APPEND structured_layout_export_targets ${component_target})
    list(APPEND structured_layout_components ${component})
    set(structured_layout_export_targets "${structured_layout_export_targets}" PARENT_SCOPE)
    set(structured_layout_components "${structured_layout_components}" PARENT_SCOPE)
    if(urd_install)
        install(DIRECTORY "${PROJECT_SOURCE_DIR}/${directory}/include/"
            DESTINATION "${component_include}")
    endif()
endfunction()

function(urd_check_properties)
    include(CheckCXXSourceCompiles)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        string(APPEND CMAKE_REQUIRED_FLAGS " -fms-extensions")
    endif()
    check_cxx_source_compiles([=[
        struct property_record {
            int value = 0;
            int get() const { return value; }
            void set(int next) { value = next; }
            __declspec(property(get = get, put = set)) int field;
        };
        int main() {
            property_record value;
            value.field = 7;
            return value.field == 7 ? 0 : 1;
        }
    ]=] urd_has_native_properties)
    if(NOT urd_has_native_properties)
        message(FATAL_ERROR "Property views require native __declspec(property) support")
    endif()
endfunction()

function(urd_check_reflection)
    if(NOT "cxx_std_26" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
        message(FATAL_ERROR "Reflected views require a C++26-capable compiler and CMake")
    endif()
    include(CheckCXXSourceCompiles)
    set(CMAKE_CXX_STANDARD 26)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        string(APPEND CMAKE_REQUIRED_FLAGS " -freflection")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        string(APPEND CMAKE_REQUIRED_FLAGS " -freflection-latest")
    endif()
    check_cxx_source_compiles([=[
        #include <meta>
        #if !defined(__cpp_impl_reflection) || __cpp_impl_reflection < 202506L
        #error Reflected views require C++26 reflection support
        #endif
        union reflected_record;
        consteval {
            std::meta::define_aggregate(^^reflected_record, {
                std::meta::data_member_spec(^^int, {.name = "value"})
            });
        }
        int main() { reflected_record value{7}; return value.value == 7 ? 0 : 1; }
    ]=] urd_has_aggregate_reflection)
    if(NOT urd_has_aggregate_reflection)
        message(FATAL_ERROR "Reflected views require <meta> and __cpp_impl_reflection >= 202506L")
    endif()
endfunction()

if(structured_layout_build_experimental)
    urd_add_component(experimental_union_named_view
        experimental/union_named_view urd::urd 23)
endif()

if(structured_layout_build_property_views)
    urd_check_properties()
    urd_add_component(property_view
        optional/property_view structured_layout::structured_layout 23)
    target_compile_options(structured_layout_property_view INTERFACE
        "$<$<CXX_COMPILER_ID:Clang,AppleClang>:-fms-extensions>")
endif()

if(structured_layout_build_reflected_views)
    urd_check_reflection()
    urd_add_component(experimental_reflected_named_view
        experimental/reflected_named_view structured_layout::structured_layout 26)
    target_compile_options(structured_layout_experimental_reflected_named_view INTERFACE
        "$<$<CXX_COMPILER_ID:GNU>:-freflection>"
        "$<$<CXX_COMPILER_ID:Clang,AppleClang>:-freflection-latest>")
endif()

if(structured_layout_build_generated_union_views)
    urd_add_component(experimental_generated_union_view
        experimental/generated_union_view structured_layout::structured_layout 23)
endif()
