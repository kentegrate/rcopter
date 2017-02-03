cmake_minimum_required(VERSION 2.8.3)
project(rcopter_drivers)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages

#add_definitions(-DHAS_BCM2835) ## comment out this when not developing on RasperryPi.

find_package(catkin REQUIRED COMPONENTS
  roscpp
  geometry_msgs
  std_msgs
  rcopter_msgs
)

set(GTEST_ROOT ~/Downloads/googletest-release-1.8.0/)
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES rcopter_drivers
#  CATKIN_DEPENDS other_catkin_pkg
#  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(include ${catkin_INCLUDE_DIRS})
include_directories(${GTEST_ROOT}/include/)
link_directories(${GTEST_ROOT}/build/)
## Declare a C++ library
if(HAS_BCM2835)
  add_library(mpu9250_driver src/mpu9250.cpp)
  target_link_libraries(mpu9250_driver bcm2835)

  add_library(ads1115_driver src/ads1115.cpp)
  target_link_libraries(ads1115_driver bcm2835)

  add_library(pca9685_driver src/pca9685.cpp)
  target_link_libraries(pca9685_driver bcm2835)

  add_library(ms5611_driver src/ms5611.cpp)
  target_link_libraries(ms5611_driver bcm2835)
endif(HAS_BCM2835)


## Add cmake target dependencies of the library
## as an example, code may need to be generated before libraries
## either from message generation or dynamic reconfigure
# add_dependencies(rcopter_drivers ${${PROJECT_NAME}_EXPORTED_TARGETS}


## Declare a C++ executable
add_executable(mpu9250_node src/mpu9250_node.cpp)
target_link_libraries(mpu9250_node ${catkin_LIBRARIES} gmock pthread)
add_dependencies(mpu9250_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS} ${rcopter_msgs_EXPORTED_TARGETS})

add_executable(ads1115_node src/ads1115_node.cpp)
target_link_libraries(ads1115_node ${catkin_LIBRARIES} gmock pthread)
add_dependencies(ads1115_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS} ${rcopter_msgs_EXPORTED_TARGETS})

add_executable(ms5611_node src/ms5611_node.cpp)
target_link_libraries(ms5611_node ${catkin_LIBRARIES} gmock pthread)
add_dependencies(ms5611_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS} ${rcopter_msgs_EXPORTED_TARGETS})

add_executable(pca9685_node src/pca9685_node.cpp)
target_link_libraries(pca9685_node ${catkin_LIBRARIES} gmock pthread)
add_dependencies(pca9685_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS} ${rcopter_msgs_EXPORTED_TARGETS})

if(HAS_BCM2835)
  target_link_libraries(mpu9250_node mpu9250_driver)
  target_link_libraries(ads1115_node ads1115_driver)  
  target_link_libraries(ms5611_node ms5611_driver)
  target_link_libraries(pca9685_node pca9685_driver)
endif(HAS_BCM2835)

