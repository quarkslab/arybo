add_library(patests SHARED tests.cpp)
target_link_libraries(patests petanque)

add_executable(op_add op_add.cpp)
target_link_libraries(op_add patests)
add_test(op_add op_add)

add_executable(op_or op_or.cpp)
target_link_libraries(op_or patests)
add_test(op_or op_or)

add_executable(op_mul op_mul.cpp)
target_link_libraries(op_mul patests)
add_test(op_mul op_mul)

add_executable(op_complex op_complex.cpp)
target_link_libraries(op_complex patests)
add_test(op_complex op_complex)

add_executable(op_esf op_esf.cpp)
target_link_libraries(op_esf patests)
add_test(op_esf op_esf)

add_executable(op_names op_names.cpp)
target_link_libraries(op_names patests)
add_test(op_names op_names)

add_executable(simp_constants_prop simp_constants_prop.cpp)
target_link_libraries(simp_constants_prop patests)
add_test(simp_constants_prop simp_constants_prop)

add_executable(simp_flatten_no_rec simp_flatten_no_rec.cpp)
target_link_libraries(simp_flatten_no_rec patests)
add_test(simp_flatten_no_rec simp_flatten_no_rec)

add_executable(simp_flatten simp_flatten.cpp)
target_link_libraries(simp_flatten patests)
add_test(simp_flatten simp_flatten)

add_executable(simp_remove_dead_ops simp_remove_dead_ops.cpp)
target_link_libraries(simp_remove_dead_ops patests)
add_test(simp_remove_dead_ops simp_remove_dead_ops)

add_executable(simp_expand_no_rec simp_expand_no_rec.cpp)
target_link_libraries(simp_expand_no_rec patests)
add_test(simp_expand_no_rec simp_expand_no_rec)

add_executable(simp_expand simp_expand.cpp)
target_link_libraries(simp_expand patests)
add_test(simp_expand simp_expand)

add_executable(simp simp.cpp)
target_link_libraries(simp patests)
add_test(simp simp)

add_executable(algos algos.cpp)
target_link_libraries(algos patests)
add_test(algos algos)

add_executable(vector vector.cpp)
target_link_libraries(vector patests)
add_test(vector vector)

add_executable(matrix matrix.cpp)
target_link_libraries(matrix patests)
add_test(matrix matrix)

add_executable(subs subs.cpp)
target_link_libraries(subs patests)
add_test(subs subs)

add_executable(vec_int vec_int.cpp)
target_link_libraries(vec_int patests)
add_test(vec_int vec_int)

add_executable(random_simplify random_simplify.cpp)
target_link_libraries(random_simplify patests)
add_test(random_simplify random_simplify 256)

add_executable(sort sort.cpp)
target_link_libraries(sort patests)
add_test(sort sort)

add_executable(sort2 sort2.cpp)
target_link_libraries(sort2 patests)
add_test(sort2 sort2)

add_executable(vectorial_decomp vectorial_decomp.cpp)
target_link_libraries(vectorial_decomp patests)
add_test(vectorial_decomp vectorial_decomp)


add_executable(prettyprinter prettyprinter.cpp)
target_link_libraries(prettyprinter patests)
add_test(prettyprinter prettyprinter)

add_executable(exprs exprs.cpp)
target_link_libraries(exprs patests)
add_test(exprs exprs)

add_executable(app app.cpp)
target_link_libraries(app patests)
add_test(app app)

add_executable(or_to_esf or_to_esf.cpp)
target_link_libraries(or_to_esf patests)
add_test(or_to_esf or_to_esf)

add_executable(identify_ors_no_rec identify_ors_no_rec.cpp)
target_link_libraries(identify_ors_no_rec patests)
add_test(identify_ors_no_rec identify_ors_no_rec)

add_executable(syms_set syms_set.cpp)
target_link_libraries(syms_set patests)
add_test(syms_set syms_set)

add_executable(visitors visitors.cpp)
target_link_libraries(visitors patests)
add_test(visitors visitors)

add_executable(sorted_vector sorted_vector.cpp)
add_test(sorted_vector sorted_vector)
