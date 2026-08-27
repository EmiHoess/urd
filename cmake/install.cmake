include(CMakePackageConfigHelpers)

install(TARGETS urd EXPORT urd_targets)
install(TARGETS ${structured_layout_export_targets} EXPORT structured_layout_targets)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/urd/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/structured_layout/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${PROJECT_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/urd")

foreach(package IN ITEMS urd structured_layout)
    set(package_directory "${CMAKE_INSTALL_LIBDIR}/cmake/${package}")
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${package}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${package}Config.cmake"
        INSTALL_DESTINATION "${package_directory}"
        NO_SET_AND_CHECK_MACRO)
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT)
    install(EXPORT ${package}_targets
        FILE ${package}Targets.cmake
        NAMESPACE ${package}::
        DESTINATION "${package_directory}")
    install(FILES
        "${PROJECT_BINARY_DIR}/${package}Config.cmake"
        "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
        DESTINATION "${package_directory}")
endforeach()
