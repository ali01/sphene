'''
Run a live test against a Sphene router running in VNS with topology 594.

                                        +====================+
                                        |                    |
                                        |   10.3.0.29        |
                                        |                    |
                                        +====================+
                                                /
                                               /
                                              /
                    eth0:                    /
                   10.3.0.24                /     eth1: 10.3.0.28
                           +============(eth1)==+
                           |                    |
  internet =============(eth0)  Your Router     |
                           |                    |
                           +============(eth2)==+
                                            \    eth2: 10.3.0.30
                                             \
                                              \
                                               \
                                        +====================+
                                        |                    |
                                        |  10.3.0.31         |
                                        |                    |
                                        +====================+
                                           Application Server
'''
from nose.tools import *
import network_lib


class Test_Topo594:
  def setUp(self):
    pass

  def test_ping_router_eth0(self):
    assert_true(network_lib.ping('10.3.0.24'))

  def test_ping_router_eth1(self):
    assert_true(network_lib.ping('10.3.0.28'))

  def test_ping_router_eth1(self):
    assert_true(network_lib.ping('10.3.0.31'))

  def test_ping_app_server1(self):
    assert_true(network_lib.ping('10.3.0.29'))

  def test_ping_app_server1(self):
    assert_true(network_lib.ping('10.3.0.31'))

  def test_traceroute_to_app_server1(self):
    trace = network_lib.traceroute('10.3.0.29')

    # Last two hops should be the router and the app server
    # itself.
    assert_true(len(trace) > 2)
    assert_equal(trace[-2], '10.3.0.24')
    assert_equal(trace[-1], '10.3.0.29')  # app server

  def test_traceroute_to_app_server2(self):
    trace = network_lib.traceroute('10.3.0.31')
    assert_true(len(trace) > 2)
    assert_equal(trace[-2], '10.3.0.24')
    assert_equal(trace[-1], '10.3.0.31')  # app server
