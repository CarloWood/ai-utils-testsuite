AM_CPPFLAGS = -iquote $(top_srcdir) -iquote $(top_srcdir)/cwds

noinst_PROGRAMS = refcount_test manip_test multiloop_test vector_test sbll_test \
		  pool_test itoa_test vtable_manip_test fuzzybool_test atomicfuzzybool_test \
		  benchmark_BitSet random_hash_test deque_allocator_test memory_test print_using

memory_test_SOURCES = memory_test.cxx
memory_test_CXXFLAGS = -O2 @LIBCWD_R_FLAGS@
memory_test_LDADD = ../utils/libutils_r.la ../cwds/libcwds_r.la -lboost_system -lboost_thread

refcount_test_SOURCES = refcount_test.cxx
refcount_test_CXXFLAGS = @LIBCWD_FLAGS@
refcount_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

manip_test_SOURCES = manip_test.cxx
manip_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
manip_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

multiloop_test_SOURCES = multiloop_test.cxx
multiloop_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
multiloop_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

vector_test_SOURCES = vector_test.cxx
vector_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
vector_test_LDADD = ../utils/libutils.la ../microbench/libmicrobench.la ../cwds/libcwds.la

sbll_test_SOURCES = sbll_test.cxx
sbll_test_CXXFLAGS = @LIBCWD_FLAGS@
sbll_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

pool_test_SOURCES = pool_test.cxx
pool_test_CXXFLAGS = @LIBCWD_FLAGS@
pool_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

itoa_test_SOURCES = itoa_test.cxx
itoa_test_CXXFLAGS = @LIBCWD_FLAGS@
itoa_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

vtable_manip_test_SOURCES = vtable_manip_test.cxx
vtable_manip_test_CXXFLAGS = @LIBCWD_FLAGS@
vtable_manip_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

fuzzybool_test_SOURCES = fuzzybool_test.cxx
fuzzybool_test_CXXFLAGS = @LIBCWD_FLAGS@
fuzzybool_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

atomicfuzzybool_test_SOURCES = atomicfuzzybool_test.cxx
atomicfuzzybool_test_CXXFLAGS = @LIBCWD_FLAGS@
atomicfuzzybool_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

benchmark_BitSet_SOURCES = benchmark_BitSet.cxx
benchmark_BitSet_CXXFLAGS = -O2 @LIBCWD_R_FLAGS@
benchmark_BitSet_LDADD = ../utils/libutils_r.la ../cwds/libcwds_r.la

random_hash_test_SOURCES = random_hash_test.cxx
random_hash_test_CXXFLAGS = -O2 @LIBCWD_R_FLAGS@
random_hash_test_LDADD = ../utils/libutils_r.la ../cwds/libcwds_r.la

deque_allocator_test_SOURCES = deque_allocator_test.cxx
deque_allocator_test_CXXFLAGS = -O2 @LIBCWD_R_FLAGS@
deque_allocator_test_LDADD = ../utils/libutils_r.la ../cwds/libcwds_r.la

print_using_SOURCES = print_using.cxx
print_using_CXXFLAGS = @LIBCWD_FLAGS@
print_using_LDADD = ../utils/libutils.la ../cwds/libcwds.la
