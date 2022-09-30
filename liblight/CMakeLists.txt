file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(light ${SRC_LIST} ${HEADERS})

target_link_libraries(light PUBLIC blockchain devcore)
