AM_CPPFLAGS = -iquote $(top_srcdir) -iquote $(top_srcdir)/cwds

noinst_PROGRAMS = refcount_test manip_test multiloop_test vector_test sbll_test

refcount_test_SOURCES = refcount_test.cxx

manip_test_SOURCES = manip_test.cxx

multiloop_test_SOURCES = multiloop_test.cxx

vector_test_SOURCES = vector_test.cxx

sbll_test_SOURCES = sbll_test.cxx

manip_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
manip_test_LDADD = ../utils/libutils.la $(top_builddir)/cwds/libcwds.la

multiloop_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
multiloop_test_LDADD = ../utils/libutils.la $(top_builddir)/cwds/libcwds.la

vector_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
vector_test_LDADD = ../utils/libutils.la ../microbench/libmicrobench.la $(top_builddir)/cwds/libcwds.la

sbll_test_CXXFLAGS = @LIBCWD_FLAGS@
sbll_test_LDADD = ../utils/libutils.la ../cwds/libcwds.la
