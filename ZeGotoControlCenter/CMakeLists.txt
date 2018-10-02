cmake_minimum_required (VERSION 3.0)
project (ZeGotoControlCenter)

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
