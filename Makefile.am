AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
GTEST_DIR = gtest-1.6.0

# MODE controls whether the router gets/sends packets from/to the NetFPGA or
# VNS
MODE_NETFPGA = -D_CPUMODE_
MODE_VNS     =
MODE         = $(MODE_VNS)

if LINUX
ARCH = -D_LINUX_
SOCK = -lnsl
endif

if DARWIN
ARCH = -D_DARWIN_
SOCK =
endif

if STANDALONE_CLI
CLI_FLAGS
endif

AM_CPPFLAGS = -m32 -O3 -Wall -Werror -D_GNU_SOURCE_ -ggdb \
              -I src -I src/cli -I src/lwtcp \
              $(ARCH) $(MODE) $(CLI_FLAGS) \
              -fno-strict-aliasing
AM_LDFLAGS  = -m32 $(SOCK) -lm -lresolv -lpthread
AM_LFLAGS   = -d -i
AM_YFLAGS   = -d


# Static libraries.
USER_LIBS = libsr_base.a liblwtcp.a
noinst_LIBRARIES = $(USER_LIBS) libgtest.a

liblwtcp_a_SOURCES = src/lwtcp/tcp.c src/lwtcp/tcp_input.c											\
                     src/lwtcp/tcp_output.c src/lwtcp/mem.c src/lwtcp/memp.c		\
                     src/lwtcp/stats.c src/lwtcp/sys.c src/lwtcp/inet.c					\
                     src/lwtcp/pbuf.c src/lwtcp/sys_arch.c src/lwtcp/sockets.c	\
                     src/lwtcp/api_lib.c src/lwtcp/api_msg.c										\
                     src/lwtcp/transport_subsys.c src/lwtcp/udp.c								\
                     src/lwtcp/icmp.c src/lwtcp/ip_addr.c src/lwtcp/err.c

libsr_base_a_SOURCES = src/sr_base.c src/sr_dumper.c src/sr_integration.c		\
                       src/sr_lwtcp_glue.c src/sr_vns.c											\
                       src/sr_cpu_extension_nf2.c src/real_socket_helper.c	\
                       src/sha1.c

# Google Test library.
libgtest_a_SOURCES = $(GTEST_DIR)/src/gtest-all.cc $(GTEST_DIR)/src/gtest_main.cc
libgtest_a_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR) -I $(GTEST_DIR)/include


# Binaries.
bin_PROGRAMS = sr
if STANDALONE_CLI
bin_PROGRAMS += cli
endif

BUILT_SOURCES = src/cli/scanner.c src/cli/parser.c

# sr
RESTRICTED_CLI_SRCS = src/cli/cli.c src/cli/cli_help.c src/cli/cli_main.c			\
                      src/cli/search_state.c src/cli/cli_local.c							\
                      src/cli/parser.y src/cli/scanner.l src/cli/helper.c
SR_SRCS_CLI = src/cli/socket_helper.c $(RESTRICTED_CLI_SRCS) src/sr_lwtcp_glue.c
SR_SRCS_MAIN = src/sr_main.c
SR_SRCS_BASE = src/nf2util.c

sr_SOURCES = $(SR_SRCS_MAIN) $(SR_SRCS_BASE) $(SR_SRCS_CLI)
sr_LDADD = $(USER_LIBS)

# cli
cli_SOURCES = $(SR_SRCS_CLI)
cli_LDADD = $(USER_LIBS)

# Unit tests.
TESTS = sample1_unittest
bin_PROGRAMS += $(TESTS)

sample1_unittest_SOURCES = tests/sample1_unittest.cc tests/sample1.cc
sample1_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
sample1_unittest_LDADD = libgtest.a


.PHONY: all deep-clean
deep-clean: distclean
	rm -f aclocal.m4 configure config.sub depcomp missing install-sh
	rm -f config.guess
	rm -rf autom4te.cache
	rm -rf */*.in *.in
