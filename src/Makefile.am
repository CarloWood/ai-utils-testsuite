AM_CPPFLAGS = -iquote $(top_srcdir) -iquote $(top_srcdir)/cwds

noinst_PROGRAMS = refcount_test manip_test multiloop_test vector_test sbll_test pool_test itoa_test derived_from_test

refcount_test_SOURCES = refcount_test.cxx
refcount_test_CXXFLAGS = @LIBCWD_FLAGS@
refcount_test_LDADD = ../utils/libutils.la $(top_builddir)/cwds/libcwds.la

manip_test_SOURCES = manip_test.cxx
manip_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
manip_test_LDADD = ../utils/libutils.la $(top_builddir)/cwds/libcwds.la

multiloop_test_SOURCES = multiloop_test.cxx
multiloop_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
multiloop_test_LDADD = ../utils/libutils.la $(top_builddir)/cwds/libcwds.la

vector_test_SOURCES = vector_test.cxx
vector_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
vector_test_LDADD = ../utils/libutils.la ../microbench/libmicrobench.la $(top_builddir)/cwds/libcwds.la

sbll_test_SOURCES = sbll_test.cxx
sbll_test_CXXFLAGS = @LIBCWD_FLAGS@
sbll_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

pool_test_SOURCES = pool_test.cxx
pool_test_CXXFLAGS = @LIBCWD_FLAGS@
pool_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

itoa_test_SOURCES = itoa_test.cxx
itoa_test_CXXFLAGS = @LIBCWD_FLAGS@
itoa_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la

derived_from_test_SOURCES = derived_from_test.cxx
derived_from_test_CXXFLAGS = @LIBCWD_FLAGS@
derived_from_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la
