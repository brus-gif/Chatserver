aux_source_directory(. CLIENT_LIST)
add_executable(Chatclient ${CLIENT_LIST})
target_link_libraries(Chatclient pthread)