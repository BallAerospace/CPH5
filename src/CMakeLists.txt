#Set them minimum version
cmake_minimum_required(VERSION 3.12)
project(cph5 C CXX)

#Set the dependencies
set(HDF5_USE_STATIC_LIBRARIES ON) 
find_package(HDF5 1.8.18 REQUIRED COMPONENTS C CXX)

#################################################################
# Create the target
#################################################################
add_library(${PROJECT_NAME} INTERFACE)

#Set an alias for use later
add_library(cph5::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

#set the warnings to empty
target_compile_options(${PROJECT_NAME} INTERFACE "")

#set the compile properties
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

#set the target include directories
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
                                                     ${HDF5_INCLUDE_DIRS})
                                                    
#set the target sources
target_sources(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cph5attribute.h
                                         ${CMAKE_CURRENT_SOURCE_DIR}/cph5comptype.h
                                         ${CMAKE_CURRENT_SOURCE_DIR}/cph5dataset.h
                                         ${CMAKE_CURRENT_SOURCE_DIR}/cph5group.h
                                         ${CMAKE_CURRENT_SOURCE_DIR}/cph5.h
                                         ${CMAKE_CURRENT_SOURCE_DIR}/cph5utilities.h
                                         ${CMAKE_CURRENT_SOURCE_DIR}/cph5varlenstr.h)
                                         
#add the external libraries it depends on 
target_link_libraries(${PROJECT_NAME} INTERFACE ${HDF5_LIBRARIES})

  
