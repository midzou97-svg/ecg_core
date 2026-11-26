cmake_minimum_required(VERSION 3.21)
project(EcgCoreSplit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QCoro6 REQUIRED COMPONENTS Core)

add_subdirectory(ecgcore)
add_subdirectory(ecgring)
add_subdirectory(ecgkrp)
add_subdirectory(app)
