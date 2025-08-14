#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.27)
project(G01)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available Vis drivers by default
# You can set WITH_GEANT4_VIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_VIS "Build example with Geant4 Vis drivers" ON)
if(WITH_GEANT4_VIS)
  find_package(Geant4 REQUIRED gdml vis_all)
else()
  find_package(Geant4 REQUIRED gdml)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${Geant4_INCLUDE_DIR}
                    )

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/G01PrimaryGeneratorAction.cc
                  ${PROJECT_SOURCE_DIR}/src/YourActionInitialization.cc
                  ${PROJECT_SOURCE_DIR}/src/YourRunAction.cc
                  ${PROJECT_SOURCE_DIR}/src/YourEventAction.cc
                  ${PROJECT_SOURCE_DIR}/src/YourSteppingAction.cc
                  ${PROJECT_SOURCE_DIR}/src/G01DetectorConstruction.cc
                  ${PROJECT_SOURCE_DIR}/src/AveragedHistogramCollection.cc
                  ${PROJECT_SOURCE_DIR}/src/HistogramCollection.cc

                  )
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/G01PrimaryGeneratorAction.hh
                    ${PROJECT_SOURCE_DIR}/include/YourActionInitialization.hh
                    ${PROJECT_SOURCE_DIR}/include/YourRunAction.hh
                    ${PROJECT_SOURCE_DIR}/include/YourEventAction.hh
                    ${PROJECT_SOURCE_DIR}/include/YourSteppingAction.hh
                    ${PROJECT_SOURCE_DIR}/include/G01DetectorConstruction.hh
                    ${PROJECT_SOURCE_DIR}/include/AveragedHistogram.hh
                    ${PROJECT_SOURCE_DIR}/include/AveragedHistogramCollection.hh
                    ${PROJECT_SOURCE_DIR}/include/HistogramCollection.hh
                    )

#----------------------------------------------------------------------------
# include ROOT dependency for histograming
#
# Find ROOT package
find_package(ROOT REQUIRED COMPONENTS Hist)
# Include ROOT headers
include_directories(${ROOT_INCLUDE_DIRS})
# Enable ROOT-specific flags (e.g. RTTI, C++17 if needed)
include(${ROOT_USE_FILE})

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(load_gdml load_gdml.cc ${sources} ${headers})
add_executable(hgcal_sim hgcal_sim.cc ${sources} ${headers})
target_link_libraries(load_gdml ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})
target_link_libraries(hgcal_sim ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})


#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build G01. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(G01_SCRIPTS
     vis.mac
  )

foreach(_script ${G01_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# Add program to the project targets
# (this avoids the need of typing the program name after make)
#
add_custom_target(G01 DEPENDS load_gdml)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS load_gdml DESTINATION bin)

