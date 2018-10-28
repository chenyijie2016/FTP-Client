cmake_minimum_required(VERSION 3.9)
project(ftpclient)

set(CMAKE_CXX_STANDARD 11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the libraries
find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Network)

add_executable(client common.cpp common.h client.cpp FtpClient.cpp FtpClient.h )
target_link_libraries(client Qt5::Widgets Qt5::Network)