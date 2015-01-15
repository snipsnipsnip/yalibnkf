cmake_minimum_required(VERSION 2.8)
project(yalibnkf C CXX)

### Common stuff

get_filename_component(CMAKELISTDIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(ROOT "${CMAKELISTDIR}" PATH)

find_program(PERL perl)
find_program(PYTHON python)

### Compiler flags

include_directories("${ROOT}" "${ROOT}/nkf" "${ROOT}/test")

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()

  if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
  endif()

  # suppress warnings in nkf.c
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4127 /wd4702 /wd4100 /wd4245")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

add_definitions(-DYALIBNKF_BUILDING)

### Static Library

add_library(yalibnkf_static STATIC "${ROOT}/yalibnkf_amalgamated.c")
set_target_properties(yalibnkf_static PROPERTIES LINKER_LANGUAGE C)

install(FILES "${ROOT}/yalibnkf.h" DESTINATION include)
install(TARGETS yalibnkf_static DESTINATION lib)

### Dynamic library

add_library(yalibnkf SHARED "${ROOT}/yalibnkf_amalgamated.c")
set_target_properties(yalibnkf PROPERTIES LINKER_LANGUAGE C)
if(MSVC)
  set_target_properties(yalibnkf PROPERTIES COMPILE_FLAGS -DYALIBNKF_DLL)
endif()
install(TARGETS yalibnkf DESTINATION bin)

### Test runners

enable_testing()

# internal version: loads test data from C arrays
add_executable(yalibnkf_test_internal "${ROOT}/test/test_internal.cpp" "${ROOT}/test/test_cases.inc")
target_link_libraries(yalibnkf_test_internal yalibnkf_static)
add_test(
  NAME test_internal
  COMMAND yalibnkf_test_internal)

# external version: loads test data from stdin/stdout
add_executable(yalibnkf_test_external "${ROOT}/test/test_external.cpp" "${ROOT}/test/test_cases.inc")
target_link_libraries(yalibnkf_test_external yalibnkf_static)
add_test(
  NAME test_external
  COMMAND ${PERL} "${ROOT}/tools/pipe_test.pl" "$<TARGET_FILE:yalibnkf_test_external>" "${ROOT}/test/test_cases.inc")

### Generated files

if(MSVC)
  set(amalgamated_source_encoding utf_8_sig)
else()
  set(amalgamated_source_encoding utf_8)
endif()

add_custom_command(
  OUTPUT "${ROOT}/yalibnkf_amalgamated.c"
  COMMAND ${PYTHON} tools/amalgamator.py
    --input-encoding=iso2022_jp
    --input-encoding=sjis
    --output-encoding=${amalgamated_source_encoding}
    --comment
    --blacklist=nkf/nkf32dll.c
    --blacklist=nkf/nkf32.h
    --out=yalibnkf_amalgamated.c
    yalibnkf.c
  WORKING_DIRECTORY "${ROOT}"
  DEPENDS "${ROOT}/tools/amalgamator.py" "${ROOT}/yalibnkf.c" "${ROOT}/yalibnkf.h" "${ROOT}/nkf/utf8tbl.c" "${ROOT}/nkf/utf8tbl.h" "${ROOT}/nkf/config.h" "${ROOT}/nkf/nkf.c" "${ROOT}/nkf/nkf.h"
  COMMENT "Generate amalgamated all-in-one source file"
  VERBATIM
)

add_custom_command(
  OUTPUT "${ROOT}/test/test_cases.inc"
  COMMAND ${PERL} tools/convert_nkf_test.pl nkf/nkf_test.pl > test/test_cases.inc
  WORKING_DIRECTORY "${ROOT}"
  DEPENDS "${ROOT}/nkf/nkf_test.pl" "${ROOT}/tools/convert_nkf_test.pl"
  COMMENT "Convert nkf_test.pl into C readable test data"
  VERBATIM
)
