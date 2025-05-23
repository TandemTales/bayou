cmake_minimum_required(VERSION 3.16)

# Add executable for the client
add_executable(BayouBonanzaClient ClientMain.cpp Client.cpp ${SHARED_SOURCES})

# Include directories
target_include_directories(BayouBonanzaClient PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link SFML libraries
target_link_libraries(BayouBonanzaClient PRIVATE sfml-graphics sfml-window sfml-system sfml-network)

# Copy assets directory to build output directory
add_custom_command(TARGET BayouBonanzaClient POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:BayouBonanzaClient>/assets)

# Copy SFML DLLs to output directory for Windows if SFML was downloaded
if(WIN32 AND DOWNLOAD_SFML)
  add_custom_command(TARGET BayouBonanzaClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:sfml-graphics>"
      "$<TARGET_FILE:sfml-window>"
      "$<TARGET_FILE:sfml-system>"
      "$<TARGET_FILE:sfml-network>"
      $<TARGET_FILE_DIR:BayouBonanzaClient>)
endif()
