cmake_minimum_required(VERSION 3.16)
project(BayouBonanza VERSION 0.1.0)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to download SFML if not found
option(DOWNLOAD_SFML "Download SFML if not found" ON)

# Option to download SQLite3 if not found
option(DOWNLOAD_SQLITE3 "Download SQLite3 if not found" ON)

# Find SFML packages
find_package(SFML 2.5 COMPONENTS graphics window system network)

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
  set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE) # Re-enabled for client
  set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)   # Re-enabled for client
  set(SFML_BUILD_NETWORK ON CACHE BOOL "" FORCE)
  
  FetchContent_MakeAvailable(SFML)
  
  message(STATUS "SFML downloaded successfully")
endif()

# Find or download SQLite3
find_package(SQLite3)
if(NOT SQLite3_FOUND AND DOWNLOAD_SQLITE3)
  include(FetchContent)
  
  message(STATUS "SQLite3 not found, downloading from GitHub...")
  
  FetchContent_Declare(
    sqlite3
    GIT_REPOSITORY https://github.com/azadkuh/sqlite-amalgamation.git
    GIT_TAG master
  )
  
  FetchContent_MakeAvailable(sqlite3)
  
  # Create SQLite3 library target
  add_library(sqlite3_lib STATIC 
    ${sqlite3_SOURCE_DIR}/sqlite3.c
  )
  
  target_include_directories(sqlite3_lib PUBLIC ${sqlite3_SOURCE_DIR})
  
  # Create an alias to match the expected target name
  add_library(SQLite::SQLite3 ALIAS sqlite3_lib)
  
  set(SQLite3_FOUND TRUE)
  set(SQLite3_LIBRARIES sqlite3_lib)
  set(SQLite3_INCLUDE_DIRS ${sqlite3_SOURCE_DIR})
  
  message(STATUS "SQLite3 downloaded and configured successfully")
endif()

# Include directories
include_directories(include)
include_directories(vendor) # Added vendor directory

# Collect source files for GameLogic library
set(GAMELOGIC_SOURCES
    src/GameBoard.cpp
    src/Square.cpp
    src/Piece.cpp
    # src/King.cpp # Removed
    # src/Queen.cpp # Removed
    # src/Rook.cpp # Removed
    # src/Bishop.cpp # Removed
    # src/Knight.cpp # Removed
    # src/Pawn.cpp # Removed
    src/PieceFactory.cpp
    src/GameState.cpp
    src/Move.cpp
    src/MoveExecutor.cpp
    src/GameRules.cpp
    src/TurnManager.cpp
    src/GameInitializer.cpp
    src/GameOverDetector.cpp
    src/CombatCalculator.cpp
    src/CombatIntegrator.cpp
    src/CombatSystem.cpp
    src/HealthTracker.cpp
    src/PieceRemovalHandler.cpp
    src/PieceDefinitionManager.cpp # Added PieceDefinitionManager.cpp
    src/InfluenceSystem.cpp # Added InfluenceSystem.cpp
)

# Add static library for game logic
add_library(GameLogic STATIC ${GAMELOGIC_SOURCES})
target_include_directories(GameLogic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link SFML to GameLogic library so it can access SFML headers
if(SFML_FOUND OR DOWNLOAD_SFML)
  target_link_libraries(GameLogic PUBLIC sfml-network sfml-system)
endif()

# Source files for executables
set(CLIENT_SOURCES 
    src/main.cpp
    src/InputManager.cpp
    src/GraphicsManager.cpp
)
set(SERVER_SOURCES src/server_main.cpp)

# Add executables
add_executable(BayouBonanzaClient ${CLIENT_SOURCES})
add_executable(BayouBonanzaServer ${SERVER_SOURCES})

# Link SQLite3 to BayouBonanzaServer
if(SQLite3_FOUND)
    message(STATUS "SQLite3 found, linking to BayouBonanzaServer.")
    target_include_directories(BayouBonanzaServer PUBLIC ${SQLite3_INCLUDE_DIRS})
    target_link_libraries(BayouBonanzaServer PRIVATE ${SQLite3_LIBRARIES})
else()
    message(WARNING "SQLite3 not found. The server might not compile or run correctly if database features are used.")
endif()

# Add gameplay test executable
add_executable(GameplayTest test_gameplay.cpp)
target_link_libraries(GameplayTest GameLogic)

# Add initialization test executable
add_executable(TestInitialization test_initialization.cpp)
target_link_libraries(TestInitialization GameLogic)

# Set Visual Studio debugger working directory to project root for client
set_target_properties(BayouBonanzaClient PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Copy assets directory to build output directory for client
add_custom_command(TARGET BayouBonanzaClient POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:BayouBonanzaClient>/assets)

# Add tests subdirectory
add_subdirectory(tests)

# Link libraries
if(SFML_FOUND OR DOWNLOAD_SFML)
  # Link client (GameLogic already includes sfml-network and sfml-system)
  target_link_libraries(BayouBonanzaClient PUBLIC GameLogic sfml-graphics sfml-window) # Added PUBLIC
  
  # Link server (GameLogic already includes sfml-network and sfml-system)
  target_link_libraries(BayouBonanzaServer PUBLIC GameLogic) # Added PUBLIC

  # TODO: Update test linking in tests/CMakeLists.txt to link against GameLogic

  # Copy SFML DLLs to output directory for Windows (for client)
  if(WIN32)
    if(DOWNLOAD_SFML)
      # If we downloaded SFML, get DLLs from the build directory for client
      add_custom_command(TARGET BayouBonanzaClient POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:sfml-graphics>"
          "$<TARGET_FILE:sfml-window>"
          "$<TARGET_FILE:sfml-system>"
          # sfml-network is not needed for client DLLs
          $<TARGET_FILE_DIR:BayouBonanzaClient>)
      
      # For server (system and network DLLs)
      add_custom_command(TARGET BayouBonanzaServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:sfml-network>"
          "$<TARGET_FILE:sfml-system>"
          $<TARGET_FILE_DIR:BayouBonanzaServer>)
    else()
      # If using system SFML, need to find the DLLs for client
      get_target_property(GRAPHICS_DLL_PATH sfml-graphics IMPORTED_LOCATION_DEBUG)
      get_filename_component(SFML_DLL_DIR_GRAPHICS ${GRAPHICS_DLL_PATH} DIRECTORY)
      
      # Client DLLs
      set(CLIENT_DLL_FILES "")
      list(APPEND CLIENT_DLL_FILES "${SFML_DLL_DIR_GRAPHICS}/sfml-graphics-2.dll") # Adjust for release/debug if necessary
      list(APPEND CLIENT_DLL_FILES "${SFML_DLL_DIR_GRAPHICS}/sfml-window-2.dll")
      list(APPEND CLIENT_DLL_FILES "${SFML_DLL_DIR_GRAPHICS}/sfml-system-2.dll")
      # Add other dependencies like openal32.dll, freetype.dll if needed, often they are in the same dir or system path

      foreach(DLL_FILE ${CLIENT_DLL_FILES})
        if(EXISTS "${DLL_FILE}")
          add_custom_command(TARGET BayouBonanzaClient POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${DLL_FILE}"
              $<TARGET_FILE_DIR:BayouBonanzaClient>)
        endif()
      endforeach()

      # Server DLLs (system and network)
      # Similar logic for server if needed, adjust sfml-network path
      get_target_property(NETWORK_DLL_PATH sfml-network IMPORTED_LOCATION_DEBUG) # Or IMPORTED_LOCATION
      get_filename_component(SFML_NETWORK_DLL_DIR ${NETWORK_DLL_PATH} DIRECTORY)
      get_target_property(SYSTEM_DLL_PATH sfml-system IMPORTED_LOCATION_DEBUG) # Or IMPORTED_LOCATION
      get_filename_component(SFML_SYSTEM_DLL_DIR ${SYSTEM_DLL_PATH} DIRECTORY) # Should be same as network

      set(SERVER_SFML_DLLS
          "${SFML_NETWORK_DLL_DIR}/sfml-network-2.dll" # Or sfml-network-d-2.dll for debug
          "${SFML_SYSTEM_DLL_DIR}/sfml-system-2.dll"   # Or sfml-system-d-2.dll for debug
      )
      foreach(DLL_FILE_PATH ${SERVER_SFML_DLLS})
        if(EXISTS "${DLL_FILE_PATH}")
             add_custom_command(TARGET BayouBonanzaServer POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_FILE_PATH}"
                $<TARGET_FILE_DIR:BayouBonanzaServer>)
        endif()
      endforeach()
    endif()
  endif()
else()
  message(FATAL_ERROR "SFML was not found and downloading is disabled. Please install SFML 2.5 or enable DOWNLOAD_SFML option.")
endif()
