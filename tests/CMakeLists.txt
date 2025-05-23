cmake_minimum_required(VERSION 3.16)

# Include the FetchContent module
include(FetchContent)

# Download Catch2
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0  # Use the latest tag/release
)
FetchContent_MakeAvailable(Catch2)

# Create test executable
add_executable(BayouBonanzaTests
  PieceTests.cpp
  KingTests.cpp
  QueenTests.cpp
  RookTests.cpp
  BishopTests.cpp
  KnightTests.cpp
  PawnTests.cpp
  PieceFactoryTests.cpp
  CombatSystemTests.cpp
)

# Include directories
target_include_directories(BayouBonanzaTests PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link against the main library and Catch2
target_link_libraries(BayouBonanzaTests PRIVATE 
  sfml-graphics 
  sfml-window 
  sfml-system
  Catch2::Catch2WithMain
)

# Copy the Catch2 DLLs to the output directory when on Windows
if(WIN32)
  add_custom_command(TARGET BayouBonanzaTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:Catch2::Catch2>"
      "$<TARGET_FILE:Catch2::Catch2WithMain>"
      $<TARGET_FILE_DIR:BayouBonanzaTests>)

  # Also copy SFML DLLs to test output directory
  add_custom_command(TARGET BayouBonanzaTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:sfml-graphics>"
      "$<TARGET_FILE:sfml-window>"
      "$<TARGET_FILE:sfml-system>"
      $<TARGET_FILE_DIR:BayouBonanzaTests>)
endif()

# Add the piece classes directly to the test target
# This ensures we can test them without creating a separate library
target_sources(BayouBonanzaTests PRIVATE
  ${CMAKE_SOURCE_DIR}/src/Piece.cpp
  ${CMAKE_SOURCE_DIR}/src/King.cpp
  ${CMAKE_SOURCE_DIR}/src/Queen.cpp
  ${CMAKE_SOURCE_DIR}/src/Rook.cpp
  ${CMAKE_SOURCE_DIR}/src/Bishop.cpp
  ${CMAKE_SOURCE_DIR}/src/Knight.cpp
  ${CMAKE_SOURCE_DIR}/src/Pawn.cpp
  ${CMAKE_SOURCE_DIR}/src/PieceFactory.cpp
  ${CMAKE_SOURCE_DIR}/src/GameBoard.cpp
  ${CMAKE_SOURCE_DIR}/src/Square.cpp
  ${CMAKE_SOURCE_DIR}/src/CombatSystem.cpp
)

# Network Integration Test Client
add_executable(NetworkIntegrationTestClient NetworkIntegrationTestClient.cpp)

target_include_directories(NetworkIntegrationTestClient PRIVATE
  ${CMAKE_SOURCE_DIR}/include # For NetworkProtocol.h, PlayerSide.h, etc.
)

target_link_libraries(NetworkIntegrationTestClient PRIVATE
  sfml-network
  # No other game logic or SFML graphics needed for this simple test client
)

# Copy SFML DLLs to test output directory for Windows for the new target
if(WIN32 AND (SFML_FOUND OR DOWNLOAD_SFML)) # Check if SFML is available
  # Determine if SFML was downloaded by FetchContent or found as system library
  if(DOWNLOAD_SFML AND TARGET sfml-network) # sfml-network target exists if downloaded
    add_custom_command(TARGET NetworkIntegrationTestClient POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:sfml-network>" # Only network is needed
        $<TARGET_FILE_DIR:NetworkIntegrationTestClient>)
  elseif(SFML_FOUND AND TARGET sfml-network) # System SFML
    # This part is more complex for system SFML, finding specific DLLs.
    # For simplicity, if using system SFML, user might need to ensure DLLs are in PATH.
    # The existing DLL copy logic for BayouBonanzaTests is more comprehensive for system SFML.
    # Let's adapt it slightly.
    get_target_property(SFML_NETWORK_LIBRARY sfml-network LOCATION)
    if(SFML_NETWORK_LIBRARY) # Check if property exists (it might be IMPORTED_LOCATION for system)
        get_filename_component(SFML_NETWORK_DLL_PATH ${SFML_NETWORK_LIBRARY} DIRECTORY)
        file(GLOB NETWORK_DLLS "${SFML_NETWORK_DLL_PATH}/sfml-network*.dll")
         add_custom_command(TARGET NetworkIntegrationTestClient POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${NETWORK_DLLS} # This might grab more than one if versioned, take care
            $<TARGET_FILE_DIR:NetworkIntegrationTestClient>)
    else() # Fallback for system SFML if the above doesn't work well
        # Try to get the imported location, which is common for find_package SFML
        get_target_property(SFML_NETWORK_IMPORTED_LOCATION sfml-network IMPORTED_LOCATION_DEBUG)
        if(NOT SFML_NETWORK_IMPORTED_LOCATION)
            get_target_property(SFML_NETWORK_IMPORTED_LOCATION sfml-network IMPORTED_LOCATION_RELEASE)
        endif()
        if(SFML_NETWORK_IMPORTED_LOCATION)
            add_custom_command(TARGET NetworkIntegrationTestClient POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SFML_NETWORK_IMPORTED_LOCATION}"
                $<TARGET_FILE_DIR:NetworkIntegrationTestClient>)
        else()
            message(WARNING "Could not determine SFML network DLL location for NetworkIntegrationTestClient system copy.")
        endif()
    endif()
  endif()
endif()
