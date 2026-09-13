cmake_minimum_required(VERSION 3.24)

project(PartsInventoryNative VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "The Windows build requires a 64-bit toolchain.")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_add_executable(parts-inventory
    src/main.cpp
    src/inventorydocument.cpp
    src/inventorydocument.h
    src/inventorywindow.cpp
    src/inventorywindow.h
    src/themesettingsdialog.cpp
    src/themesettingsdialog.h
)

target_link_libraries(parts-inventory PRIVATE
    Qt6::Widgets
    Qt6::Network
)

target_compile_definitions(parts-inventory PRIVATE
    PARTS_INVENTORY_VERSION="${PROJECT_VERSION}"
)

install(TARGETS parts-inventory
    RUNTIME DESTINATION bin
)

if(WIN32)
    qt_generate_deploy_app_script(
        TARGET parts-inventory
        OUTPUT_SCRIPT deploy_script
        NO_TRANSLATIONS
    )
    install(SCRIPT ${deploy_script})

    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(runtime_dll IN ITEMS libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
        if(EXISTS "${MINGW_BIN_DIR}/${runtime_dll}")
            install(FILES "${MINGW_BIN_DIR}/${runtime_dll}" DESTINATION bin)
        endif()
    endforeach()
endif()

set(CPACK_PACKAGE_NAME "parts-inventory")
set(CPACK_PACKAGE_VENDOR "Parts Inventory")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Parts Inventory")
    set(CPACK_NSIS_PACKAGE_NAME "Parts Inventory")
else()
    set(CPACK_GENERATOR "DEB")
endif()
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Parts Inventory")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
include(CPack)
