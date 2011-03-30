AC_PREREQ(2.61)
AC_INIT([router], [1.0], msparks@stanford.edu)
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_YACC
AM_PROG_LIBTOOL
AM_PROG_LEX

# Option to build the standalone CLI.
AC_ARG_ENABLE(
  [standalone_cli],
  [  --enable-standalone-cli     Build standalone CLI ],
  [case "${enableval}" in
     yes) standalone_cli=yes ;;
     no)  standalone_cli=no ;;
     *) AC_MSG_ERROR([bad value ${enableval} for --enable-standalone-cli]) ;;
   esac],
  [standalone_cli=no])
AM_CONDITIONAL([STANDALONE_CLI], [test x$standalone_cli = xyes])

# Determine the operating system.
AC_MSG_CHECKING([uname -s for detecting host operating system])
OSNAME=`uname -s`
case "$OSNAME" in
  Linux*)
    AC_MSG_RESULT(Linux)
    linux=yes
  ;;
  Darwin*)
    AC_MSG_RESULT(Darwin)
    darwin=yes
  ;;
esac
AM_CONDITIONAL(LINUX, test x$linux = xyes)
AM_CONDITIONAL(DARWIN, test x$darwin = xyes)

#AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([
  Makefile
  ])
AC_OUTPUT

echo
echo "Building standalone cli: $standalone_cli"
echo