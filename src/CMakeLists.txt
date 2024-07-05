find_package(QT NAMES Qt6 Qt5 COMPONENTS
    Core
    Network
    WebSockets
    REQUIRED)

project(Qt${QT_VERSION_MAJOR}Buttplug VERSION ${QBP_VERSION_MAJOR}.${QBP_VERSION_MINOR}.${QBP_VERSION_PATCH} LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
  Core
  Network
  WebSockets
  REQUIRED)

set(QBP_SRC
  qbuttplugclient.cpp
  qbuttplugclient_p.cpp
  qbuttplugclient_p.h
  qbuttplugclientdevice.cpp
  qbuttplugclientdevice_p.cpp
  qbuttplugclientdevice_p.h
  qbuttplugmessageparsing.cpp)

if (${QBP_BUILD_TYPE} STREQUAL STATIC)
  add_library(${PROJECT_NAME} STATIC ${QBP_SRC})
else()
  add_library(${PROJECT_NAME} SHARED ${QBP_SRC})
endif()

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} BASE_NAME qbuttplug)

if (QBP_DEBUG)
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME Qt${QT_VERSION_MAJOR}Buttplugd)
endif()

target_include_directories(${PROJECT_NAME} INTERFACE "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")
target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")
#target_include_directories(${PROJECT_NAME} PRIVATE ${Qt${QT_VERSION_MAJOR}Core_PRIVATE_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Network
      Qt${QT_VERSION_MAJOR}::WebSockets
)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /WX)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_FEATURE_zstd=-1)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES
  qbuttplugclient.h QButtplugClient
  qbuttplugclientdevice.h QButtplugClientDevice
  QButtplugMessageParsing qbuttplugenums.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})
