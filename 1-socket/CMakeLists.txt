cmake_minimum_required(VERSION 3.22)
project(socket)

set(CMAKE_C_STANDARD 17)

add_executable(simplex_client simplex_client.c)
add_executable(simplex_server simplex_server.c)
add_executable(duplex_client duplex_client.c)
add_executable(duplex_server duplex_server.c)
add_executable(saw_client saw_client.c)
add_executable(saw_server saw_server.c)

target_link_libraries(simplex_client PRIVATE wsock32)
target_link_libraries(simplex_server PRIVATE wsock32)
target_link_libraries(duplex_client PRIVATE wsock32)
target_link_libraries(duplex_server PRIVATE wsock32)
target_link_libraries(saw_client PRIVATE wsock32)
target_link_libraries(saw_server PRIVATE wsock32)
