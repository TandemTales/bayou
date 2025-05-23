cmake_minimum_required(VERSION 3.16)

# Add executable for the server
add_executable(BayouBonanzaServer ServerMain.cpp Server.cpp ${SHARED_SOURCES})

# Include directories
target_include_directories(BayouBonanzaServer PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link SFML libraries
target_link_libraries(BayouBonanzaServer PRIVATE sfml-network sfml-graphics sfml-window sfml-system)

# Copy SFML DLLs to output directory for Windows if SFML was downloaded
if(WIN32 AND DOWNLOAD_SFML)
  add_custom_command(TARGET BayouBonanzaServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:sfml-network>"
      "$<TARGET_FILE:sfml-graphics>"
      "$<TARGET_FILE:sfml-window>"
      "$<TARGET_FILE:sfml-system>"
      $<TARGET_FILE_DIR:BayouBonanzaServer>)
endif()
