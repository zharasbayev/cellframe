cmake_minimum_required(VERSION 3.10)
project (dap_chain_net_srv_app_db)
  
file(GLOB DAP_CHAIN_NET_SRV_APP_DB_SRCS *.c)

file(GLOB DAP_CHAIN_NET_SRV_APP_DB_HEADERS include/*.h)

add_library(${PROJECT_NAME} STATIC ${DAP_CHAIN_NET_SRV_APP_DB_SRCS} ${DAP_CHAIN_NET_SRV_APP_DB_HEADERS})

target_link_libraries(${PROJECT_NAME} dap_chain_net_srv dap_chain_net_srv_app)
target_include_directories(${PROJECT_NAME} INTERFACE .)
target_include_directories(${PROJECT_NAME} PUBLIC include)

