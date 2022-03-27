include(AICxxProject)

add_executable(u8string_to_filename_test u8string_to_filename_test.cxx)
target_link_libraries(u8string_to_filename_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(pointer_hash_test pointer_hash_test.cxx)
target_link_libraries(pointer_hash_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(color_pool_test color_pool_test.cxx)
target_link_libraries(color_pool_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(register_test register_test.cxx)
target_link_libraries(register_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(c_escape_test c_escape_test.cxx)
target_link_libraries(c_escape_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(dictionary_test dictionary_test.cxx)
target_link_libraries(dictionary_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(atomicfuzzybool_test atomicfuzzybool_test.cxx)
target_link_libraries(atomicfuzzybool_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(benchmark_BitSet benchmark_BitSet.cxx)
if (CW_BUILD_TYPE_IS_DEBUG)
  target_compile_options(benchmark_BitSet PRIVATE "-O2")
endif()
target_link_libraries(benchmark_BitSet PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(deque_allocator_test deque_allocator_test.cxx)
target_link_libraries(deque_allocator_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(fuzzybool_test fuzzybool_test.cxx)
target_link_libraries(fuzzybool_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(is_pointer_like is_pointer_like.cxx)
target_link_libraries(is_pointer_like PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(itoa_test itoa_test.cxx)
target_link_libraries(itoa_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(manip_test manip_test.cxx)
target_link_libraries(manip_test PRIVATE ${AICXX_OBJECTS_LIST})

find_package(Boost COMPONENTS thread)
if (Boost_THREAD_FOUND)
  add_executable(memory_test memory_test.cxx)
  if (CW_BUILD_TYPE_IS_DEBUG)
    target_compile_options(memory_test PRIVATE "-O2")
  endif()
  target_link_libraries(memory_test PRIVATE ${AICXX_OBJECTS_LIST} Boost::thread)
endif()

add_executable(multiloop_test multiloop_test.cxx)
target_link_libraries(multiloop_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(pool_test pool_test.cxx)
target_link_libraries(pool_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(print_using print_using.cxx)
target_link_libraries(print_using PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(random_hash_test random_hash_test.cxx)
target_link_libraries(random_hash_test PRIVATE ${AICXX_OBJECTS_LIST})

add_executable(refcount_test refcount_test.cxx)
target_link_libraries(refcount_test PRIVATE ${AICXX_OBJECTS_LIST})

if (TARGET MoodyCamel::microbench)
  add_executable(vector_test vector_test.cxx)
  target_link_libraries(vector_test PRIVATE ${AICXX_OBJECTS_LIST} MoodyCamel::microbench)
endif ()

add_executable(vtable_manip_test vtable_manip_test.cxx)
target_link_libraries(vtable_manip_test PRIVATE ${AICXX_OBJECTS_LIST})
