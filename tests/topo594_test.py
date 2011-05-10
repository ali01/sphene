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
import urllib2

import network_lib


class Test_Topo594:
  def setUp(self):
    self._big_photo_size = 1053791  # bytes
    self._rtr_eth0 = '10.3.0.24'
    self._rtr_eth1 = '10.3.0.28'
    self._rtr_eth2 = '10.3.0.30'
    self._app1 = '10.3.0.29'
    self._app2 = '10.3.0.31'

  def test_ping_router_eth0(self):
    '''Ping eth0 on router'''
    assert_true(network_lib.ping(self._rtr_eth0))

  def test_ping_router_eth1(self):
    '''Ping eth1 on router'''
    assert_true(network_lib.ping(self._rtr_eth0))

  def test_ping_router_eth2(self):
    '''Ping eth2 on router'''
    assert_true(network_lib.ping(self._rtr_eth2))

  def test_ping_app_server1(self):
    '''Ping app server 1'''
    assert_true(network_lib.ping(self._app1))

  def test_ping_app_server2(self):
    '''Ping app server 2'''
    assert_true(network_lib.ping(self._app2))

  def test_traceroute_to_app_server1(self):
    '''Traceroute to app server 1'''
    trace = network_lib.traceroute(self._app1)

    # Last two hops should be the router and the app server
    # itself.
    assert_true(len(trace) > 2)
    assert_equal(trace[-2], self._rtr_eth0)
    assert_equal(trace[-1], self._app1)

  def test_traceroute_to_app_server2(self):
    '''Traceroute to app server 2'''
    trace = network_lib.traceroute(self._app2)
    assert_true(len(trace) > 2)
    assert_equal(trace[-2], self._rtr_eth0)
    assert_equal(trace[-1], self._app2)

  def test_http_app_server1_basic(self):
    '''Test basic HTTP on app server 1'''
    url = urllib2.urlopen('http://%s' % self._app1)
    content = url.read()
    assert_true('application server' in content)

  def test_http_app_server2_basic(self):
    '''Test basic HTTP on app server 2'''
    url = urllib2.urlopen('http://%s' % self._app2)
    content = url.read()
    assert_true('application server' in content)

  def test_http_app_server1_big(self):
    '''Fetch big photo from app server 1'''
    url = urllib2.urlopen('http://%s/big.jpg' % self._app1)
    content = url.read()
    assert_equal(len(content), self._big_photo_size)

  def test_http_app_server2_big(self):
    '''Fetch big photo from app server 2'''
    url = urllib2.urlopen('http://%s/big.jpg' % self._app2)
    content = url.read()
    assert_equal(len(content), self._big_photo_size)
