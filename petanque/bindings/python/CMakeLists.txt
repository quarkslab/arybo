include_directories(${PYTHON_INCLUDE_DIRS})

set(PYTHON_SRC_FILES
	pytanque.cpp
)

add_library(pytanque SHARED ${PYTHON_SRC_FILES})
set_target_properties(pytanque PROPERTIES PREFIX "") # We need our ".so" to be named "pytanque" (same name as in the BOOST_PYTHON_MODULE macro)
set_target_properties(pytanque PROPERTIES
  VISIBILITY_INLINES_HIDDEN ON
  C_VISIBILITY_PRESET hidden
  CXX_VISIBILITY_PRESET hidden)

target_link_libraries(pytanque PRIVATE petanque ${PYTHON_LIBRARIES} ${Boost_LIBRARIES} ${TBB_LIBRARIES})

execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())" OUTPUT_VARIABLE PYTHON_INSTALL OUTPUT_STRIP_TRAILING_WHITESPACE)
install(TARGETS pytanque LIBRARY DESTINATION ${PYTHON_INSTALL}/ ARCHIVE DESTINATION ${PYTHON_INSTALL}/)
