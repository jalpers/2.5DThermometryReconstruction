cmake_minimum_required (VERSION 2.6)
project (thermometry_reconstruction)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

# Set Local Debugger Settings (Command Arguments and Environment Variables)
set( COMMAND_ARGUMENTS "commands" )
# Change the following line to point to your paths of QT and VTK 8.2.0
set( ENVIRONMENT_VARIABLES "C:\\Qt\\5.12.1\\msvc2015_64\\bin;D:\\Repository\\2_5DThermometryReconstruction\\dependencies\\VTK-8.2.0\\build\\bin\\Release" )
configure_file( template.vcxproj.user.in ${CMAKE_BINARY_DIR}/thermometry_reconstruction.vcxproj.user @ONLY )

set (HEADER ${HEADER}
	../include/ThermometryReconstruction.h
	../include/CoordinatesConverter.h
	../include/DicomHandler.h
	../include/DataVolume.h
	../include/InterpolationMap.h
)

set (SOURCE ${SOURCE}
	../sources/main.cpp
	../sources/ThermometryReconstruction.cpp
	../sources/CoordinatesConverter.cpp
	../sources/DicomHandler.cpp
	../sources/DataVolume.cpp
	../sources/InterpolationMap.cpp
)

set (UI ${UI}
	../sources/ThermometryReconstruction.ui
)

source_group("Source" FILES ${SOURCE})
source_group("Header" FILES ${HEADER})
source_group("UI" FILES ${UI})

#add QT5 dependencies 	
find_package(Qt5 COMPONENTS Core Gui Network WebSockets Widgets REQUIRED)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(thermometry_reconstruction ${SOURCE} ${HEADER} ${UI})

target_link_libraries(thermometry_reconstruction 
	Qt5::Widgets 
	Qt5::Core
	Qt5::Network
	Qt5::WebSockets
	Qt5::Gui
	${VTK_LIBRARIES}
)