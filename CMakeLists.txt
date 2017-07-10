cmake_minimum_required(VERSION 2.8.11)
project(hlsplay)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Multimedia REQUIRED)

set(hlsplay_SRC
  src/main.cpp
  src/hlsplay.cpp
  src/stream.cpp
)

# Create code from a list of Qt designer ui files.
#set(CMAKE_AUTOUIC ON) # use this if you have CMake 3.x instead of the following
qt5_wrap_ui(hlsplay_SRC src/hlsplay.ui)

# Tell CMake to create the helloworld executable
add_executable(hlsplay ${hlsplay_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(hlsplay Qt5::Widgets Qt5::Network Qt5::Multimedia)

# Install the executable
install(TARGETS hlsplay DESTINATION bin)
