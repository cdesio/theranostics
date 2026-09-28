cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(PhotonDetector)

option(WITH_GEANT4_VIS "Build with Geant4 UI/visualization support" ON)

# Find Geant4 package
if(WITH_GEANT4_VIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()
# Include Geant4 directories
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${Geant4_INCLUDE_DIR})


# Gather sources and headers
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

# Copy macro and data files to the build directory
file(GLOB MACRO_FILES "${PROJECT_SOURCE_DIR}/*.mac")
file(COPY ${MACRO_FILES} DESTINATION ${PROJECT_BINARY_DIR})

file(GLOB DATA_FILES "${PROJECT_SOURCE_DIR}/*.dat")
file(COPY ${DATA_FILES} DESTINATION ${PROJECT_BINARY_DIR})

# Create the executable
add_executable(PhotonDetector main.cc ${sources} ${headers})

if(WITH_GEANT4_VIS)
  target_compile_definitions(PhotonDetector PRIVATE ALPHAGLUE_USE_VIS=1)
endif()

# Link Geant4 libraries
target_link_libraries(PhotonDetector ${Geant4_LIBRARIES})

# Add custom target
add_custom_target(PhotonDetector_l DEPENDS PhotonDetector)
