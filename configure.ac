# Define package version here.
define(CW_VERSION_MAJOR, 1)
define(CW_VERSION_MINOR, 0)
define(CW_VERSION_REVISION, 0)
define(CW_PACKAGE_NAME, [ai-utils-testsuite])

# Other configuration.
define(CW_BUGREPORT, [carlo@alinoe.com])
define(CW_COMPILER_WARNINGS, [-W -Wall -Woverloaded-virtual -Wundef -Wpointer-arith -Wwrite-strings -Winline])

# Include cwm4 header.
m4_include([cwm4/configure_ac_top.m4])

dnl We use doxygen for documentation.
CW_DOXYGEN

AC_CONFIG_FILES([src/Makefile])

# Include cwm4 footer.
m4_include([cwm4/configure_ac_bottom.m4])
