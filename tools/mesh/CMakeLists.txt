include_directories(${CMAKE_CURRENT_BINARY_DIR})

function(CHANGE_EXT var ext)
  set(names "")
  foreach(f ${ARGN})
    get_filename_component(FILE_WITHOUT_EXT ${f} NAME_WE)
    list(APPEND names "${FILE_WITHOUT_EXT}.${ext}")
  endforeach(f)
  set(${var} "${names}" PARENT_SCOPE)
endfunction(CHANGE_EXT)

file(GLOB MODELS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.json)
file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)

CHANGE_EXT(MODEL_AC_H "ac.h" ${MODELS})
CHANGE_EXT(MODEL_AC_CPP "ac.cpp" ${MODELS})

add_custom_command(
  DEPENDS ${MODELS}
  OUTPUT  ${MODEL_AC_H} ${MODEL_AC_CPP}
  COMMAND ${CMAKE_COMMAND} -E env ace-compile -I ${CMAKE_CURRENT_SOURCE_DIR} ${MODELS}
  COMMENT "Compiling test models"
  VERBATIM
  )

add_executable(mesh ${SOURCES} ${MODEL_AC_CPP})
target_compile_features(mesh PRIVATE cxx_nullptr)
target_link_libraries(mesh PUBLIC ${ACE_LIBRARIES})
