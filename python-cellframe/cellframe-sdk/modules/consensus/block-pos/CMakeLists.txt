cmake_minimum_required(VERSION 3.10)
project (dap_chain_cs_block_pos)
  
file(GLOB DAP_CHAIN_CS_BLOCK_POS_SRCS *.c)
file(GLOB DAP_CHAIN_CS_BLOCK_POS_HEADERS include/*.h)

add_library(${PROJECT_NAME} STATIC ${DAP_CHAIN_CS_BLOCK_POS_SRCS} ${DAP_CHAIN_CS_BLOCK_POS_HEADERS})

target_link_libraries(${PROJECT_NAME} dap_core dap_crypto dap_chain dap_chain_cs_blocks dap_chain_net_srv_stake_pos_delegate)
target_include_directories(${PROJECT_NAME} INTERFACE .)
target_include_directories(${PROJECT_NAME} PUBLIC include)
