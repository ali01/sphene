AUTOMAKE_OPTIONS = foreign color-tests
ACLOCAL_AMFLAGS = -I m4
GTEST_DIR = tests/gtest

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

liblwtcp_a_SOURCES = src/lwtcp/tcp.cc src/lwtcp/tcp_input.cc			\
                     src/lwtcp/tcp_output.cc src/lwtcp/mem.cc src/lwtcp/memp.cc	\
                     src/lwtcp/stats.cc src/lwtcp/sys.cc src/lwtcp/inet.cc	\
                     src/lwtcp/pbuf.cc src/lwtcp/sys_arch.cc			\
                     src/lwtcp/sockets.cc src/lwtcp/api_lib.cc			\
                     src/lwtcp/api_msg.cc src/lwtcp/transport_subsys.cc		\
                     src/lwtcp/udp.cc src/lwtcp/icmp.cc src/lwtcp/ip_addr.cc	\
                     src/lwtcp/err.cc

cc_srcs = src/data_plane.cc

libsr_base_a_SOURCES = src/sr_base.cc src/sr_dumper.cc src/sr_integration.cc	\
                       src/sr_lwtcp_glue.cc src/sr_vns.cc			\
                       src/sr_cpu_extension_nf2.cc src/real_socket_helper.cc	\
                       src/sha1.cc $(cc_srcs)

# Google Test library.
libgtest_a_SOURCES = $(GTEST_DIR)/src/gtest-all.cc $(GTEST_DIR)/src/gtest_main.cc
libgtest_a_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR) -I $(GTEST_DIR)/include


# Binaries.
bin_PROGRAMS = sr
if STANDALONE_CLI
bin_PROGRAMS += cli
endif

BUILT_SOURCES = src/cli/scanner.cc src/cli/parser.cc

# sr
RESTRICTED_CLI_SRCS = src/cli/cli.cc src/cli/cli_help.cc src/cli/cli_main.cc	\
                      src/cli/search_state.cc src/cli/cli_local.cc		\
                      src/cli/parser.yy src/cli/scanner.ll src/cli/helper.cc
SR_SRCS_CLI = src/cli/socket_helper.cc $(RESTRICTED_CLI_SRCS) src/sr_lwtcp_glue.cc
SR_SRCS_MAIN = src/sr_main.cc
SR_SRCS_BASE = src/nf2util.cc

sr_SOURCES = $(SR_SRCS_MAIN) $(SR_SRCS_BASE) $(SR_SRCS_CLI)
sr_LDADD = $(USER_LIBS)

# cli
cli_SOURCES = $(SR_SRCS_CLI)
cli_LDADD = $(USER_LIBS)

# Unit tests.
TESTS = sample1_unittest packet_unittest
bin_PROGRAMS += $(TESTS)

sample1_unittest_SOURCES = tests/sample1_unittest.cc tests/sample1.cc
sample1_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
sample1_unittest_LDADD = libgtest.a

packet_unittest_SOURCES = tests/packet_unittest.cc
packet_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
packet_unittest_LDADD = libgtest.a $(USER_LIBS)

.PHONY: all deep-clean
deep-clean: distclean
	rm -f aclocal.m4 configure config.sub depcomp missing install-sh
	rm -f config.guess
	rm -rf autom4te.cache
	rm -rf */*.in *.in
