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
