cmake_minimum_required(VERSION 3.16)
project(BayouBonanza VERSION 0.1.0)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to download SFML if not found
option(DOWNLOAD_SFML "Download SFML if not found" ON)

# Find SFML packages
find_package(SFML 2.5 COMPONENTS graphics window system)

# If SFML is not found and download is enabled, use FetchContent to download it
if(NOT SFML_FOUND AND DOWNLOAD_SFML)
  include(FetchContent)

  message(STATUS "SFML not found, downloading...")
  
  FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.5.1
  )
  
  # Disable unneeded components to speed up the build
  set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
  
  FetchContent_MakeAvailable(SFML)
  
  message(STATUS "SFML downloaded successfully")
endif()

# Include directories
include_directories(include)

# Collect source files
set(SOURCES
    src/main.cpp
    src/GameBoard.cpp
    src/Square.cpp
    src/Piece.cpp
    src/King.cpp
    src/Queen.cpp
    src/Rook.cpp
    src/Bishop.cpp
    src/Knight.cpp
    src/Pawn.cpp
    src/PieceFactory.cpp
    src/GameState.cpp
    src/Move.cpp
    src/MoveExecutor.cpp
    src/GameRules.cpp
    src/TurnManager.cpp
    src/GameInitializer.cpp
    src/GameOverDetector.cpp
)

# Add executable
add_executable(BayouBonanza ${SOURCES})

# Set Visual Studio debugger working directory to project root
set_target_properties(BayouBonanza PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Copy assets directory to build output directory
add_custom_command(TARGET BayouBonanza POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:BayouBonanza>/assets)

# Add tests subdirectory
add_subdirectory(tests)

# Link libraries
if(SFML_FOUND OR DOWNLOAD_SFML)
  target_link_libraries(BayouBonanza sfml-graphics sfml-window sfml-system)
  
  # Copy SFML DLLs to output directory for Windows
  if(WIN32)
    if(DOWNLOAD_SFML)
      # If we downloaded SFML, get DLLs from the build directory
      add_custom_command(TARGET BayouBonanza POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:sfml-graphics>"
          "$<TARGET_FILE:sfml-window>"
          "$<TARGET_FILE:sfml-system>"
          $<TARGET_FILE_DIR:BayouBonanza>)
    else()
      # If using system SFML, need to find the DLLs
      # Get the directory where SFML DLLs are located
      get_target_property(GRAPHICS_DLL_PATH sfml-graphics IMPORTED_LOCATION_DEBUG)
      get_filename_component(SFML_DLL_DIR ${GRAPHICS_DLL_PATH} DIRECTORY)
      
      # Copy all SFML DLLs to the output directory
      file(GLOB SFML_DLLS "${SFML_DLL_DIR}/*.dll")
      foreach(DLL ${SFML_DLLS})
        add_custom_command(TARGET BayouBonanza POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}"
            $<TARGET_FILE_DIR:BayouBonanza>)
      endforeach()
    endif()
  endif()
else()
  message(FATAL_ERROR "SFML was not found and downloading is disabled. Please install SFML 2.5 or enable DOWNLOAD_SFML option.")
endif()
