AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
GTEST_DIR = tests/gtest
CK_DIR = src/ck

# MODE controls whether the router gets/sends packets from/to the NetFPGA or
# VNS
if CPUMODE
MODE = -D_CPUMODE_
else
MODE =
endif

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

if REF_REG_DEFINES
REF_REG_DEFINES = -DREF_REG_DEFINES
endif

# Force debug build.
CXXFLAGS += -O0

AM_CXXFLAGS = -m32
AM_CPPFLAGS = -march=i586 -Wall -Werror -D_GNU_SOURCE_ -ggdb \
              -D__STDC_LIMIT_MACROS \
              -I src -I src/cli -I src/lwtcp -I $(CK_DIR)/include \
              $(ARCH) $(MODE) $(CLI_FLAGS) $(REF_REG_DEFINES) \
              -fno-strict-aliasing
AM_LDFLAGS  = -m32 $(SOCK) -lrt -lm -lresolv -lpthread -L$(CK_DIR)/src -lck \
              -Wl,--no-undefined
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
           src/fwk/buffer.h \
           src/fwk/exception.h \
           src/fwk/log.cc \
           src/fwk/log.h \
           src/fwk/ptr.h \
           src/fwk/ptr_interface.h \
           src/fwk/utility.cc

libsr_base_a_SOURCES = \
                       src/arp_cache.cc \
                       src/arp_cache_daemon.cc \
                       src/arp_cache_daemon.h \
                       src/arp_packet.cc \
                       src/arp_packet.h \
                       src/arp_queue.cc \
                       src/arp_queue_daemon.cc \
                       src/arp_queue_daemon.h \
                       src/buffer.h \
                       src/control_plane.cc \
                       src/control_plane.h \
                       src/data_plane.cc \
                       src/data_plane.h \
                       src/ethernet_packet.cc \
                       src/ethernet_packet.h \
                       src/getarg.cc \
                       src/gre_packet.cc \
                       src/gre_packet.h \
                       src/hw_data_plane.cc \
                       src/hw_data_plane.h \
                       src/icmp_packet.cc \
                       src/icmp_packet.h \
                       src/interface.cc \
                       src/interface.h \
                       src/interface_map.cc \
                       src/interface_map.h \
                       src/ip_packet.cc \
                       src/ip_packet.h \
                       src/ipv4_addr.cc \
                       src/nf2.h \
                       src/nf2util.cc \
                       src/nf2util.h \
                       src/ospf_adv_map.cc \
                       src/ospf_constants.cc \
                       src/ospf_daemon.cc \
                       src/ospf_daemon.h \
                       src/ospf_gateway.cc \
                       src/ospf_interface.cc \
                       src/ospf_interface_map.cc \
                       src/ospf_link.cc \
                       src/ospf_node.cc \
                       src/ospf_packet.cc \
                       src/ospf_router.cc \
                       src/ospf_router_id.cc \
                       src/ospf_topology.cc \
                       src/packet.cc \
                       src/packet.h \
                       src/packet_buffer.h \
                       src/real_socket_helper.cc \
                       src/real_socket_helper.h \
                       src/reg_defines.h \
                       src/router.cc \
                       src/router.h \
                       src/routing_table.cc \
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
                       src/sw_data_plane.cc \
                       src/sw_data_plane.h \
                       src/task.cc \
                       src/task.h \
                       src/tunnel.cc \
                       src/tunnel.h \
                       src/tunnel_map.cc \
                       src/tunnel_map.h \
                       src/unknown_packet.cc \
                       src/unknown_packet.h \
                       src/vnscommand.h \
                       $(FWK_SRCS)

# Google Test library.
libgtest_a_SOURCES = \
                     $(GTEST_DIR)/include/gtest/gtest-death-test.h \
                     $(GTEST_DIR)/include/gtest/gtest.h \
                     $(GTEST_DIR)/include/gtest/gtest-message.h \
                     $(GTEST_DIR)/include/gtest/gtest-param-test.h \
                     $(GTEST_DIR)/include/gtest/gtest_pred_impl.h \
                     $(GTEST_DIR)/include/gtest/gtest-printers.h \
                     $(GTEST_DIR)/include/gtest/gtest_prod.h \
                     $(GTEST_DIR)/include/gtest/gtest-spi.h \
                     $(GTEST_DIR)/include/gtest/gtest-test-part.h \
                     $(GTEST_DIR)/include/gtest/gtest-typed-test.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-death-test-internal.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-filepath.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-internal.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-linked_ptr.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-param-util-generated.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-param-util.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-port.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-string.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-tuple.h \
                     $(GTEST_DIR)/include/gtest/internal/gtest-type-util.h \
                     $(GTEST_DIR)/src/gtest-all.cc \
                     $(GTEST_DIR)/src/gtest.cc \
                     $(GTEST_DIR)/src/gtest-death-test.cc \
                     $(GTEST_DIR)/src/gtest-filepath.cc \
                     $(GTEST_DIR)/src/gtest-internal-inl.h \
                     $(GTEST_DIR)/src/gtest_main.cc \
                     $(GTEST_DIR)/src/gtest-port.cc \
                     $(GTEST_DIR)/src/gtest-printers.cc \
                     $(GTEST_DIR)/src/gtest-test-part.cc \
                     $(GTEST_DIR)/src/gtest-typed-test.cc
libgtest_a_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR) -I $(GTEST_DIR)/include


# Binaries.
bin_PROGRAMS = sr
if STANDALONE_CLI
bin_PROGRAMS += cli
endif

BUILT_SOURCES = src/cli/scanner.cc src/cli/parser.cc

# sr
RESTRICTED_CLI_SRCS = src/cli/cli.cc \
                      src/cli/cli.h \
                      src/cli/cli_help.cc \
                      src/cli/cli_help.h \
                      src/cli/cli_local.cc \
                      src/cli/cli_local.h \
                      src/cli/cli_main.cc \
                      src/cli/cli_main.h \
                      src/cli/cli_parser.h \
                      src/cli/cli_scanner.h \
                      src/cli/cli_stubs.cc \
                      src/cli/cli_stubs.h \
                      src/cli/helper.cc \
                      src/cli/helper.h \
                      src/cli/parser.yy \
                      src/cli/scanner.ll \
                      src/cli/search_state.cc \
                      src/cli/search_state.h
SR_SRCS_CLI = \
              src/cli/cli_network.h \
              src/cli/socket_helper.cc \
              src/cli/socket_helper.h \
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
        arp_cache_unittest \
        arp_packet_unittest \
        atomic_unittest \
        buffer_unittest \
        ethernet_packet_unittest \
        gre_packet_unittest \
        icmp_packet_unittest \
        interface_unittest \
        interface_map_unittest \
        ip_packet_unittest \
        ospf_adv_map_unittest \
        ospf_topology_unittest \
        packet_unittest \
        packet_buffer_unittest \
        routing_table_unittest \
        sw_data_plane_unittest \
        task_unittest \
        tunnel_unittest \
        tunnel_map_unittest

bin_PROGRAMS += $(TESTS)

arp_cache_unittest_SOURCES = tests/arp_cache_unittest.cc $(FWK_SRCS)
arp_cache_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
arp_cache_unittest_LDADD = libgtest.a $(USER_LIBS)

arp_packet_unittest_SOURCES = tests/arp_packet_unittest.cc $(FWK_SRCS)
arp_packet_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
arp_packet_unittest_LDADD = libgtest.a $(USER_LIBS)

atomic_unittest_SOURCES = tests/atomic_unittest.cc $(FWK_SRCS)
atomic_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
atomic_unittest_LDADD = libgtest.a -L$(CK_DIR)/src -lck

buffer_unittest_SOURCES = tests/buffer_unittest.cc
buffer_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
buffer_unittest_LDADD = libgtest.a

ethernet_packet_unittest_SOURCES = tests/ethernet_packet_unittest.cc $(FWK_SRCS)
ethernet_packet_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
ethernet_packet_unittest_LDADD = libgtest.a $(USER_LIBS)

gre_packet_unittest_SOURCES = tests/gre_packet_unittest.cc $(FWK_SRCS)
gre_packet_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
gre_packet_unittest_LDADD = libgtest.a $(USER_LIBS)

icmp_packet_unittest_SOURCES = tests/icmp_packet_unittest.cc $(FWK_SRCS)
icmp_packet_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
icmp_packet_unittest_LDADD = libgtest.a $(USER_LIBS)

interface_unittest_SOURCES = tests/interface_unittest.cc $(FWK_SRCS)
interface_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
interface_unittest_LDADD = libgtest.a $(USER_LIBS)

interface_map_unittest_SOURCES = tests/interface_map_unittest.cc $(FWK_SRCS)
interface_map_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
interface_map_unittest_LDADD = libgtest.a $(USER_LIBS)

ip_packet_unittest_SOURCES = tests/ip_packet_unittest.cc
ip_packet_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
ip_packet_unittest_LDADD = libgtest.a $(USER_LIBS)

ospf_adv_map_unittest_SOURCES = tests/ospf_adv_map_unittest.cc $(FWK_SRCS)
ospf_adv_map_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
ospf_adv_map_unittest_LDADD = libgtest.a $(USER_LIBS)

ospf_topology_unittest_SOURCES = tests/ospf_topology_unittest.cc $(FWK_SRCS)
ospf_topology_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
ospf_topology_unittest_LDADD = libgtest.a $(USER_LIBS)

packet_unittest_SOURCES = tests/packet_unittest.cc
packet_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
packet_unittest_LDADD = libgtest.a $(USER_LIBS)

packet_buffer_unittest_SOURCES = tests/packet_buffer_unittest.cc $(FWK_SRCS)
packet_buffer_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
packet_buffer_unittest_LDADD = libgtest.a

routing_table_unittest_SOURCES = tests/routing_table_unittest.cc
routing_table_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
routing_table_unittest_LDADD = libgtest.a $(USER_LIBS)

sw_data_plane_unittest_SOURCES = tests/sw_data_plane_unittest.cc \
                                 $(FWK_SRCS) \
                                 $(SR_SRCS_CLI)
sw_data_plane_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
sw_data_plane_unittest_LDADD = libgtest.a $(USER_LIBS)

task_unittest_SOURCES = tests/task_unittest.cc src/task.cc
task_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
task_unittest_LDADD = libgtest.a

tunnel_unittest_SOURCES = tests/tunnel_unittest.cc $(FWK_SRCS)
tunnel_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
tunnel_unittest_LDADD = libgtest.a $(USER_LIBS)

tunnel_map_unittest_SOURCES = tests/tunnel_map_unittest.cc $(FWK_SRCS)
tunnel_map_unittest_CPPFLAGS = $(AM_CPPFLAGS) -I $(GTEST_DIR)/include
tunnel_map_unittest_LDADD = libgtest.a $(USER_LIBS)

.PHONY: all deep-clean
deep-clean: distclean
	rm -f aclocal.m4 configure config.sub depcomp missing install-sh
	rm -f config.guess
	rm -rf autom4te.cache
	rm -rf */*.in *.in
