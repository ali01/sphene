AC_PREREQ(2.59)
AC_INIT([router], [1.0], msparks@stanford.edu)
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_YACC
AM_PROG_LIBTOOL
AM_PROG_LEX

# Less verbose compiling.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

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

# Option to build in "cpu mode" (use NetFPGA).
AC_ARG_ENABLE(
  [cpumode],
  [  --enable-cpumode     Build in CPU mode (use NetFPGA) ],
  [case "${enableval}" in
     yes) cpumode=yes ;;
     no)  cpumode=no ;;
     *) AC_MSG_ERROR([bad value ${enableval} for --enable-cpumode]) ;;
   esac],
  [cpumode=no])
AM_CONDITIONAL([CPUMODE], [test x$cpumode = xyes])

# Option to use the reference reg_defines.h for the reference bitfile.
AC_ARG_ENABLE(
  [ref_reg_defines],
  [  --enable-ref-reg-defines     Use reference reg_defines.h ],
  [case "${enableval}" in
     yes) ref_reg_defines=yes ;;
     no)  ref_reg_defines=no ;;
     *) AC_MSG_ERROR([bad value ${enableval} for --enable-ref-reg-defines]) ;;
   esac],
  [ref_reg_defines=no])
AM_CONDITIONAL([REF_REG_DEFINES], [test x$ref_reg_defines = xyes])


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
echo "CPU mode:                $cpumode"
echo "Reference reg_defines:   $ref_reg_defines"
echo