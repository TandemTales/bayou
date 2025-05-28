cmake_minimum_required(VERSION 3.16)
include(FetchContent) # Ensures FetchContent module is available
enable_testing()      # Enables CTest for this directory and subdirectories

# --- Catch2 Integration ---
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
# FetchContent_MakeAvailable ensures catch2_SOURCE_DIR is populated
# and Catch2::Catch2WithMain target is available.
FetchContent_MakeAvailable(Catch2)

# --- Test Executable Definition ---
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
  TurnManagerTests.cpp
)
target_include_directories(BayouBonanzaTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(BayouBonanzaTests PRIVATE
  GameLogic  # Link against the GameLogic library instead of individual source files
  sfml-graphics sfml-window sfml-system sfml-network  # Added sfml-network
  Catch2::Catch2WithMain # Link against Catch2's main
)

# --- CTest Integration with Catch2 ---
# Diagnostic message to check if catch2_SOURCE_DIR is set
if(DEFINED catch2_SOURCE_DIR AND EXISTS "${catch2_SOURCE_DIR}/extras/Catch.cmake")
  message(STATUS "Catch2 source directory found: ${catch2_SOURCE_DIR}")
  # include(${catch2_SOURCE_DIR}/extras/Catch.cmake) # Temporarily commented out
  # Catch2_add_tests(BayouBonanzaTests BayouBonanzaTests) # Temporarily commented out
  message(WARNING "Catch2_add_tests and include of Catch.cmake are temporarily commented out to allow project compilation.")
  add_test(NAME BayouBonanzaAllTests_Fallback COMMAND BayouBonanzaTests) # Keep fallback for now
else()
  message(WARNING "Catch2 source directory or Catch.cmake not found. catch2_SOURCE_DIR='${catch2_SOURCE_DIR}'. CTest tests may not be discovered.")
  # Fallback to basic CTest registration if Catch2_add_tests is not available
  add_test(NAME BayouBonanzaAllTests_Fallback COMMAND BayouBonanzaTests) # Ensure fallback is present here too
endif()
