cmake_minimum_required(VERSION 3.10)
project(dap_enc_server C)
  
file(GLOB DAP_ENC_SERVER_SRCS FILES *.c)
file(GLOB DAP_ENC_SERVER_HDRS FILES include/*.h)

if(WIN32)
    add_definitions ("-DUNDEBUG")
    add_definitions ("-DNDEBUG")
    add_definitions ("-DWIN32")
    add_definitions ("-D_WINDOWS")
    add_definitions ("-D__WINDOWS__")
    add_definitions ("-D_CRT_SECURE_NO_WARNINGS")
endif()

add_library(${PROJECT_NAME} STATIC ${DAP_ENC_SERVER_SRCS} ${DAP_ENC_SERVER_HDRS})
target_include_directories(${PROJECT_NAME} INTERFACE . include/)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rdparty/uthash/src)

if(WIN32)
  target_link_libraries(${PROJECT_NAME} dap_core dap_crypto dap_server_core dap_http_server dap_json-c
  kernel32
  user32
  shell32
  winmm
  gdi32
  advapi32
  ole32
  version
  imm32
  oleaut32
  ws2_32
  ntdll
  psapi
  shlwapi
  bcrypt
  crypt32
  secur32
)
endif()

if(UNIX)
  target_link_libraries(${PROJECT_NAME} dap_core dap_crypto dap_server_core dap_http_server dap_json-c)
endif()

