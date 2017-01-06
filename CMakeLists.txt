cmake_minimum_required(VERSION 2.8.8)
project(sprinter)
find_package(Qt5Widgets REQUIRED)

file(GLOB sprinter_SOURCES
    src/*.cpp
    )

# headers that needs to be processed by moc
file(GLOB sprinter_HEADERS
    src/*.h
    )

file(GLOB sprinter_FORMS
    ui/*.ui
    )

set(sprinter_RESOURCES resources/resources.qrc)

include_directories(${CMAKE_BINARY_DIR} src/include)

include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Widgets_DEFINITIONS})

set(CMAKE_AUTOMOC ON)
qt5_wrap_ui(sprinter_FORMS_HEADERS ${sprinter_FORMS})
qt5_add_resources(sprinter_RESOURCES_RCC ${sprinter_RESOURCES})

add_executable(sprinter
    ${sprinter_SOURCES}
    ${sprinter_HEADERS_MOC}
    ${sprinter_FORMS_HEADERS}
    ${sprinter_RESOURCES_RCC}
    )

qt5_use_modules(sprinter Widgets ${sprinter_Qt5_Modules})

target_link_libraries(sprinter
    ${QT_LIBRARIES}
    ${sprinter_LIBRARIES}
    )

install(TARGETS sprinter DESTINATION bin)

