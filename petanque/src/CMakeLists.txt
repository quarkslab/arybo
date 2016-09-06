set(SRC_FILES
	analyses.cpp
	app.cpp
	bitfield.cpp
	exprs.cpp
	matrix.cpp
	ops.cpp
	prettyprinter.cpp
	simps.cpp
	subs.cpp
	symbols.cpp
	syms_hist.cpp
	syms_set.cpp
	vector.cpp
)

set(HEADER_DIST_FILES
	../include/pa/analyses.h
	../include/pa/app.h
	../include/pa/bitfield.h
	../include/pa/exprs.h
	../include/pa/matrix.h
	../include/pa/prettyprinter.h
	../include/pa/subs.h
	../include/pa/symbols.h
	../include/pa/syms_hist.h
	../include/pa/syms_set.h
	../include/pa/traits.h
	../include/pa/vector.h
)

add_library(petanque STATIC SHARED ${SRC_FILES})
set(LINK_LIBRARIES ${TBB_LIBRARIES})
target_link_libraries(petanque ${LINK_LIBRARIES})

install(TARGETS petanque LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${HEADER_DIST_FILES} DESTINATION include/pa)
