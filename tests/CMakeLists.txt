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

# --- Client-specific Test Executable ---
add_executable(BayouBonanzaClientTests
  InputManagerTests.cpp
  ${CMAKE_SOURCE_DIR}/src/InputManager.cpp  # Include InputManager source as part of the executable
)
target_include_directories(BayouBonanzaClientTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(BayouBonanzaClientTests PRIVATE
  GameLogic  # Link against the GameLogic library
  sfml-graphics sfml-window sfml-system sfml-network  # Full SFML for client tests
  Catch2::Catch2WithMain # Link against Catch2's main
)

# --- CTest Integration with Catch2 ---
# Diagnostic message to check if catch2_SOURCE_DIR is set
if(DEFINED catch2_SOURCE_DIR AND EXISTS "${catch2_SOURCE_DIR}/extras/Catch.cmake")
  message(STATUS "Catch2 source directory found: ${catch2_SOURCE_DIR}")
  # include(${catch2_SOURCE_DIR}/extras/Catch.cmake) # Temporarily commented out
  # Catch2_add_tests(BayouBonanzaTests BayouBonanzaTests) # Temporarily commented out
  # Catch2_add_tests(BayouBonanzaClientTests BayouBonanzaClientTests) # Temporarily commented out
  message(WARNING "Catch2_add_tests and include of Catch.cmake are temporarily commented out to allow project compilation.")
  add_test(NAME BayouBonanzaAllTests_Fallback COMMAND BayouBonanzaTests) # Keep fallback for now
  add_test(NAME BayouBonanzaClientTests_Fallback COMMAND BayouBonanzaClientTests) # Keep fallback for now
else()
  message(WARNING "Catch2 source directory or Catch.cmake not found. catch2_SOURCE_DIR='${catch2_SOURCE_DIR}'. CTest tests may not be discovered.")
  # Fallback to basic CTest registration if Catch2_add_tests is not available
  add_test(NAME BayouBonanzaAllTests_Fallback COMMAND BayouBonanzaTests) # Ensure fallback is present here too
  add_test(NAME BayouBonanzaClientTests_Fallback COMMAND BayouBonanzaClientTests) # Ensure fallback is present here too
endif()

# --- Database Test Executable ---
add_executable(DatabaseTests DatabaseTests.cpp)
# Find SQLite3 package - this should ideally be done at the top-level CMakeLists.txt
# but for self-contained test addition, we can try to find it here too.
# If not found, it assumes sqlite3 is available in a way the linker can find it (e.g. system path)
find_package(SQLite3)
if(SQLite3_FOUND)
  target_include_directories(DatabaseTests PRIVATE ${SQLite3_INCLUDE_DIRS}) # Added for consistency
  target_link_libraries(DatabaseTests PRIVATE ${SQLite3_LIBRARIES}) # Changed from SQLite3::SQLite3
  message(STATUS "DatabaseTests linked with SQLite3_LIBRARIES")
else()
  # Fallback for environments where FindSQLite3.cmake might not create the imported target,
  # or if SQLite3 was not found by find_package.
  # This often works if libsqlite3-dev is installed system-wide.
  target_link_libraries(DatabaseTests PRIVATE sqlite3)
  message(WARNING "SQLite3_FOUND is false. Attempting to link DatabaseTests with 'sqlite3' directly. Ensure libsqlite3-dev is installed.")
endif()

# Add DatabaseTests to CTest (simple registration)
add_test(NAME Database_Tests COMMAND DatabaseTests)
