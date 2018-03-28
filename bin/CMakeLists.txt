include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_BINARY_DIR}/lib)

add_executable(dag dag.cpp)
target_compile_features(dag PRIVATE cxx_nullptr)
target_link_libraries(dag PUBLIC ${ACE_LIBRARIES})

add_executable(composer composer.cpp)
target_compile_features(composer PRIVATE cxx_nullptr)
target_link_libraries(composer PRIVATE mesh PUBLIC ${ACE_LIBRARIES})

add_executable(node node.cpp)
target_compile_features(node PRIVATE cxx_nullptr)
target_link_libraries(node PRIVATE mesh PUBLIC ${ACE_LIBRARIES})

install(
  TARGETS composer node
  RUNTIME DESTINATION bin
  )
