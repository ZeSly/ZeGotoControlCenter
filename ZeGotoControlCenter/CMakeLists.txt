cmake_minimum_required (VERSION 3.0)
project (ZeGotoControlCenter)

SET(MAJOR_VERSION "1")
SET(MINOR_VERSION "0")
SET(PATCH_VERSION "0")

find_package(Qt5 COMPONENTS Widgets Network SerialPort)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (SRC
ascom_tcp_server.cpp
goto_tab.cpp
GPSStatusFrame.cpp
link.cpp
location_tab.cpp
main.cpp
meridian_tab.cpp
park_tab.cpp
rates_tab.cpp
singleapplication.cpp
SkyPosition.cpp
zegotocontrolcenter.cpp
)

set (HEADERS
build_number.h
resource.h
SkyPosition.h
)

set (MOC_HEADERS
GPSStatusFrame.h
link.h
singleapplication.h
zegotocontrolcenter.h
)

set (DATA_FILES
Resources/messier.csv
Resources/messiers.csv
Resources/ngc_ic.csv
Resources/stars.csv
)

set (UIS zegotocontrolcenter.ui)
set(RESSOURCE zegotocontrolcenter.qrc)

set_property(SOURCE zegotocontrolcenter.qrc PROPERTY SKIP_AUTOMOC ON)

qt5_wrap_cpp(MOC_HDR ${MOC_HEADERS}) 
qt5_wrap_ui(UIS_HDR ${UIS})
qt5_add_resources(RESSOURCE_HDR ${RESSOURCE})

if (WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif(WIN32)

add_executable (${PROJECT_NAME} ${SRC} ${HEADERS} ${MOC_HDR} ${UIS_HDR} ${RESSOURCE_HDR})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::SerialPort)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} Version)
endif(WIN32)

# add_custom_command(
    # TARGET ${PROJECT_NAME} POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E copy_if_different
        # $<TARGET_FILE:Qt5::Core>
        # $<TARGET_FILE:Qt5::Widgets>
        # $<TARGET_FILE:Qt5::Network>
        # $<TARGET_FILE:Qt5::SerialPort>
        # $<TARGET_FILE_DIR:ZeGotoControlCenter>
# )

if (WIN32)
    get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
    set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
       COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} --no-system-d3d-compiler --no-translations $<TARGET_FILE_DIR:ZeGotoControlCenter>)
endif(WIN32)

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
install (FILES ${DATA_FILES} DESTINATION lib/${PROJECT_NAME})

SET(CPACK_GENERATOR "DEB")
set(CMAKE_INSTALL_PREFIX "/usr/local")
SET(CPACK_PACKAGE_DESCRIPTION "ZeGoto Control Center")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ZeGoto Control Center for my Takahashi EM200")
SET(CPACK_PACKAGE_VENDOR "ZeSly")
SET(CPACK_PACKAGE_CONTACT "zesly42@@gmail.com")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.27), qt5-default (>= 5.5), libqt5serialport5 (>= 5.5)")

INCLUDE(CPack) 

