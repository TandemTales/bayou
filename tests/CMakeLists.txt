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
  # KingTests.cpp # Removed
  # QueenTests.cpp # Removed
      # SweetykinsTests.cpp # Removed
  # BishopTests.cpp # Removed
  # KnightTests.cpp # Removed
  # PawnTests.cpp # Removed
  PieceFactoryTests.cpp
  CombatSystemTests.cpp
  TurnManagerTests.cpp
  InfluenceSystemTests.cpp
  ResourceSystemTests.cpp
  CardTests.cpp  # Added comprehensive card system tests
  GameRulesTests.cpp  # Added comprehensive win condition tests
  StunTests.cpp
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
  ${CMAKE_SOURCE_DIR}/src/GraphicsManager.cpp  # Include GraphicsManager source as InputManager depends on it
)
target_include_directories(BayouBonanzaClientTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(BayouBonanzaClientTests PRIVATE
  GameLogic  # Link against the GameLogic library
  sfml-graphics sfml-window sfml-system sfml-network  # Full SFML for client tests
  Catch2::Catch2WithMain # Link against Catch2's main
)

# --- Database Test Executable ---
add_executable(DatabaseTests DatabaseTests.cpp)
# Use the same SQLite3 setup as the main project
if(TARGET sqlite3_lib)
  target_include_directories(DatabaseTests PRIVATE ${sqlite3_SOURCE_DIR})
  target_link_libraries(DatabaseTests PRIVATE sqlite3_lib)
  message(STATUS "DatabaseTests linked with sqlite3_lib from FetchContent")
else()
  # Fallback to system SQLite3 if available
  find_package(SQLite3)
  if(SQLite3_FOUND)
    target_include_directories(DatabaseTests PRIVATE ${SQLite3_INCLUDE_DIRS})
    target_link_libraries(DatabaseTests PRIVATE ${SQLite3_LIBRARIES})
    message(STATUS "DatabaseTests linked with system SQLite3")
  else()
    target_link_libraries(DatabaseTests PRIVATE sqlite3)
    message(WARNING "SQLite3_FOUND is false. Attempting to link DatabaseTests with 'sqlite3' directly. Ensure libsqlite3-dev is installed.")
  endif()
endif()

# Add DatabaseTests to CTest (simple registration)
add_test(NAME Database_Tests COMMAND DatabaseTests)

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

# --- Copy DLLs for Tests on Windows ---
if(WIN32)
  # Copy Catch2 DLLs to test output directories
  add_custom_command(TARGET BayouBonanzaTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:Catch2::Catch2>"
      "$<TARGET_FILE:Catch2::Catch2WithMain>"
      $<TARGET_FILE_DIR:BayouBonanzaTests>)
      
  add_custom_command(TARGET BayouBonanzaClientTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:Catch2::Catch2>"
      "$<TARGET_FILE:Catch2::Catch2WithMain>"
      $<TARGET_FILE_DIR:BayouBonanzaClientTests>)

  # Copy SFML DLLs to test output directories
  add_custom_command(TARGET BayouBonanzaTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:sfml-graphics>"
      "$<TARGET_FILE:sfml-window>"
      "$<TARGET_FILE:sfml-system>"
      "$<TARGET_FILE:sfml-network>"
      $<TARGET_FILE_DIR:BayouBonanzaTests>)
      
  add_custom_command(TARGET BayouBonanzaClientTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:sfml-graphics>"
      "$<TARGET_FILE:sfml-window>"
      "$<TARGET_FILE:sfml-system>"
      "$<TARGET_FILE:sfml-network>"
      $<TARGET_FILE_DIR:BayouBonanzaClientTests>)
      
  # Copy assets directory to test output directories
  add_custom_command(TARGET BayouBonanzaTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:BayouBonanzaTests>/assets)
      
  add_custom_command(TARGET BayouBonanzaClientTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:BayouBonanzaClientTests>/assets)
endif()
