file(GLOB SHINKUAN_HEADERS
  inc/shinkuan/*.h
  inc/shinkuan/*.hpp
)

file(GLOB SHINKUAN_SOURCES
  src/shinkuan/*.c
  src/shinkuan/*.cc
  src/shinkuan/*.cpp
)

add_library(shinkuan STATIC
  ${SHINKUAN_HEADERS}
  ${SHINKUAN_SOURCES}
)

target_include_directories(shinkuan
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${ODB_HOME}/include       # access to odb/db.h
)

target_compile_features(shinkuan PUBLIC cxx_std_17)

target_link_libraries(shinkuan
  PUBLIC
    odb
)