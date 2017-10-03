AM_CPPFLAGS = -iquote $(top_srcdir) -iquote $(top_srcdir)/cwds

noinst_PROGRAMS = refcount_test manip_test multiloop_test

refcount_test_SOURCES = refcount_test.cxx

manip_test_SOURCES = manip_test.cxx

multiloop_test_SOURCES = multiloop_test.cxx

manip_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
manip_test_LDADD = ../utils/libutils.la $(top_builddir)/cwds/libcwds.la

multiloop_test_CXXFLAGS = -fmax-errors=1 @LIBCWD_FLAGS@
multiloop_test_LDADD = ../utils/libutils.la $(top_builddir)/cwds/libcwds.la
