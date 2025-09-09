cmake_minimum_required(VERSION 3.16)
 
project(RemoteMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Network Core REQUIRED)

add_executable(RemoteMonitor
        src/client/AddMachine.h
        src/client/AddMachine.cpp
        src/client/main.cpp
        src/client/client.h
        src/client/client.cpp
        src/client/client.ui
        src/client/MachineFrame.h
        src/client/MachineFrame.cpp
        src/client/MachineFrame.ui
        src/client/ProcessFrame.h
        src/client/ProcessFrame.cpp
        src/client/processframe.ui
        resources/resources.qrc
)

add_executable(server
        src/server/Server.h
        src/server/Server.cpp
        src/server/Thread.h
        src/server/Thread.cpp
        src/server/main.cpp
)

qt_add_resources(RESOURCE_SOURCES resources/resources.qrc)
target_sources(RemoteMonitor PRIVATE ${RESOURCE_SOURCES})
  
include(FetchContent)
fetchcontent_declare(jwt-cpp 
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG 08bcf77a687fb06e34138e9e9fa12a4ecbe12332 # v0.7.0 release
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "disable building examples" FORCE)
fetchcontent_makeavailable(jwt-cpp)

target_link_libraries(RemoteMonitor PRIVATE Qt6::Widgets Qt6::Network Qt6::Core jwt-cpp::jwt-cpp)
target_link_libraries(server PRIVATE Qt6::Widgets Qt6::Network jwt-cpp::jwt-cpp)

set_target_properties(RemoteMonitor PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS RemoteMonitor 
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
