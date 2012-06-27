cmake_minimum_required(VERSION 2.8)
PROJECT(sprinter)
FIND_PACKAGE(Qt4 REQUIRED)

FILE(GLOB sprinter_SOURCES
    src/*.cpp
    )

# headers that needs to be processed by moc
FILE(GLOB sprinter_HEADERS
    src/*.h
    )

FILE(GLOB sprinter_FORMS
    ui/*.ui
    )

SET(sprinter_RESOURCES resources/resources.qrc)

INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} src/include)
ADD_DEFINITIONS(${QT_DEFINITIONS})

QT4_WRAP_CPP(sprinter_HEADERS_MOC ${sprinter_HEADERS})
QT4_WRAP_UI(sprinter_FORMS_HEADERS ${sprinter_FORMS})
QT4_ADD_RESOURCES(sprinter_RESOURCES_RCC ${sprinter_RESOURCES})

ADD_EXECUTABLE(sprinter
    ${sprinter_SOURCES}
    ${sprinter_HEADERS_MOC}
    ${sprinter_FORMS_HEADERS}
    ${sprinter_RESOURCES_RCC}
    )
TARGET_LINK_LIBRARIES(sprinter
    ${QT_LIBRARIES}
    ${sprinter_LIBRARIES}
    )

install(TARGETS sprinter DESTINATION bin)

