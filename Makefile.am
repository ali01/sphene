AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
GTEST_DIR = tests/gtest
CK_DIR = src/ck

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

AM_CPPFLAGS = -m32 -march=i586 -O3 -Wall -Werror -D_GNU_SOURCE_ -ggdb \
              -D__STDC_LIMIT_MACROS \
              -I src -I src/cli -I src/lwtcp -I $(CK_DIR)/include \
              $(ARCH) $(MODE) $(CLI_FLAGS) \
              -fno-strict-aliasing
AM_LDFLAGS  = -m32 $(SOCK) -lm -lresolv -lpthread -L$(CK_DIR)/src -lck
AM_LFLAGS   = -d -i
AM_YFLAGS   = -d


# Static libraries.
USER_LIBS = libsr_base.a liblwtcp.a
noinst_LIBRARIES = $(USER_LIBS) libgtest.a

liblwtcp_a_SOURCES = \
                     src/lwtcp/api_lib.cc \
                     src/lwtcp/api_msg.cc \
                     src/lwtcp/err.cc \
                     src/lwtcp/icmp.cc \
                     src/lwtcp/inet.cc \
                     src/lwtcp/ip_addr.cc \
                     src/lwtcp/lwip/api.h \
                     src/lwtcp/lwip/api_msg.h \
                     src/lwtcp/lwip/arch.h \
                     src/lwtcp/lwip/cc.h \
                     src/lwtcp/lwip/debug.h \
                     src/lwtcp/lwip/def.h \
                     src/lwtcp/lwip/err.h \
                     src/lwtcp/lwip/icmp.h \
                     src/lwtcp/lwip/inet.h \
                     src/lwtcp/lwip/ip_addr.h \
                     src/lwtcp/lwip/ip.h \
                     src/lwtcp/lwip/list.h \
                     src/lwtcp/lwip/lwipopts.h \
                     src/lwtcp/lwip/mem.h \
                     src/lwtcp/lwip/memp.h \
                     src/lwtcp/lwip/netif.h \
                     src/lwtcp/lwip/opt.h \
                     src/lwtcp/lwip/pbuf.h \
                     src/lwtcp/lwip/sockets.h \
                     src/lwtcp/lwip/stats.h \
                     src/lwtcp/lwip/sys.h \
                     src/lwtcp/lwip/tcp.h \
                     src/lwtcp/lwip/tcpip.h \
                     src/lwtcp/lwip/transport_subsys.h \
                     src/lwtcp/lwip/udp.h \
                     src/lwtcp/lwtcp_sr_integration.h \
                     src/lwtcp/mem.cc \
                     src/lwtcp/memp.cc \
                     src/lwtcp/pbuf.cc \
                     src/lwtcp/sockets.cc \
                     src/lwtcp/stats.cc \
                     src/lwtcp/sys.cc \
                     src/lwtcp/sys_arch.cc \
                     src/lwtcp/tcp.cc \
                     src/lwtcp/tcp_input.cc \
                     src/lwtcp/tcp_output.cc \
                     src/lwtcp/transport_subsys.cc \
                     src/lwtcp/udp.cc

FWK_SRCS = \
           src/fwk/atomic.h \
           src/fwk/exception.h \
           src/fwk/ptr.h \
           src/fwk/ptr_interface.h

libsr_base_a_SOURCES = \
                       src/arp_packet.h \
                       src/buffer.h \
                       src/data_plane.cc \
                       src/data_plane.h \
                       src/ethernet_packet.h \
                       src/getarg.cc \
                       src/icmp_packet.h \
                       src/ip_packet.h \
                       src/nf2.h \
                       src/nf2util.cc \
                       src/nf2util.h \
                       src/packet.h \
                       src/real_socket_helper.cc \
                       src/real_socket_helper.h \
                       src/reg_defines.h \
                       src/sha1.cc \
                       src/sha1.h \
                       src/sr_base.cc \
                       src/sr_base.h \
                       src/sr_base_internal.h \
                       src/sr_cpu_extension_nf2.cc \
                       src/sr_cpu_extension_nf2.h \
                       src/sr_dumper.cc \
                       src/sr_dumper.h \
                       src/sr_integration.cc \
                       src/sr_integration.h \
                       src/sr_lwtcp_glue.cc \
                       src/sr_main.cc \
                       src/sr_vns.cc \
                       src/sr_vns.h \
                       $(FWK_SRCS)

# Google Test library.
libgtest_a_SOURCES = \
                     $(GTEST_DIR)/include/gtest/gtest.h \
                     $(GTEST_DIR)/src/gtest-all.cc \
                     $(GTEST_DIR)/src/gtest_main.cc
libgtest_a_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR) -I $(GTEST_DIR)/include


# Binaries.
bin_PROGRAMS = sr
if STANDALONE_CLI
bin_PROGRAMS += cli
endif

BUILT_SOURCES = src/cli/scanner.cc src/cli/parser.cc

# sr
RESTRICTED_CLI_SRCS = src/cli/cli.cc \
                      src/cli/cli_help.cc \
                      src/cli/cli_local.cc \
                      src/cli/cli_main.cc \
                      src/cli/helper.cc \
                      src/cli/parser.yy \
                      src/cli/scanner.ll \
                      src/cli/search_state.cc
SR_SRCS_CLI = \
              src/cli/socket_helper.cc \
              $(RESTRICTED_CLI_SRCS) \
              src/sr_lwtcp_glue.cc
SR_SRCS_MAIN = src/sr_main.cc
SR_SRCS_BASE = src/nf2util.cc

sr_SOURCES = $(SR_SRCS_MAIN) $(SR_SRCS_BASE) $(SR_SRCS_CLI)
sr_LDADD = $(USER_LIBS)

# cli
cli_SOURCES = $(SR_SRCS_CLI)
cli_LDADD = $(USER_LIBS)

# Unit tests.
TESTS = \
        atomic_unittest \
        sample1_unittest \
        packet_unittest

bin_PROGRAMS += $(TESTS)

atomic_unittest_SOURCES = tests/atomic_unittest.cc $(FWK_SRCS)
atomic_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
atomic_unittest_LDADD = libgtest.a -L$(CK_DIR)/src -lck

sample1_unittest_SOURCES = \
                           tests/sample1.cc \
                           tests/sample1.h \
                           tests/sample1_unittest.cc
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
