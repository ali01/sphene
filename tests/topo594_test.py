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
import getpass
import os
import subprocess
import sys
import time
import urllib2

import network_lib
import spheneinstance

# Global instances to avoid setup and teardown for each test.
instance = None


def base_dir():
  '''Returns the base path of the sphene project directory.'''
  tests_dir = os.path.dirname(os.path.abspath(__file__))
  return os.path.normpath(os.path.join(tests_dir, '..'))


class Test_Topo594:
  def setUp(self):
    self._topo_id = 594
    self._big_photo_size = 1053791  # bytes
    self._cli_port = 2300
    self._rtr_eth0 = '10.3.0.24'
    self._rtr_eth1 = '10.3.0.28'
    self._rtr_eth2 = '10.3.0.30'
    self._app1 = '10.3.0.29'
    self._app2 = '10.3.0.31'

    # Set up an instance if it doesn't exist yet.
    if instance is None:
      self._reset()

  def _reset(self):
    '''Re-bootstrap.'''
    global instance

    # Destruct existing instance.
    instance = None

    # Should we bootstrap the system with our own sphene instance?
    if os.getenv('BOOTSTRAP', 1):
      cli_port = 23000
      instance = self._bootstrap(cli_port, pass_rtable=True)

  def _bootstrap(self, cli_port, pass_rtable=False):
    '''Initialize a sphene instance for tests.'''
    auth_key_file = os.path.join(base_dir(), 'auth_key')
    if pass_rtable:
      rtable_file = os.path.join(base_dir(), 'rtable_%d' % self._topo_id)
    else:
      rtable_file = os.path.join(base_dir(), 'rtable_empty')
    return spheneinstance.SpheneInstance(self._topo_id, cli_port, auth_key_file,
                                         rtable_file=rtable_file)

    # Wait for interface to come up.
    time.sleep(1)
    for i in range(5):
      if network_lib.ping(self._rtr_eth0):
        return

    # Failed to bootstrap.
    raise Exception('timeout waiting for router to come up')

  def test_ping_router_interfaces(self):
    '''Ping router interfaces'''
    assert_true(network_lib.ping(self._rtr_eth0))
    assert_true(network_lib.ping(self._rtr_eth1))
    assert_true(network_lib.ping(self._rtr_eth2))

  def test_ping_app_servers(self):
    '''Ping app servers'''
    assert_true(network_lib.ping(self._app1))
    assert_true(network_lib.ping(self._app2))

  def test_traceroute_to_app_server1(self):
    '''Traceroute to app server 1'''
    trace = network_lib.traceroute(self._app1)

    # Last two hops should be the router and the app server itself.
    assert_true(len(trace) > 2)
    assert_equal(trace[-2], self._rtr_eth0)
    assert_equal(trace[-1], self._app1)

  def test_traceroute_to_app_server2(self):
    '''Traceroute to app server 2'''
    trace = network_lib.traceroute(self._app2)
    assert_true(len(trace) > 2)
    assert_equal(trace[-2], self._rtr_eth0)
    assert_equal(trace[-1], self._app2)

  def test_http_app_servers_basic(self):
    '''Test basic HTTP on app servers'''
    url = urllib2.urlopen('http://%s' % self._app1, timeout=3)
    content = url.read()
    assert_true('application server' in content)

    url = urllib2.urlopen('http://%s' % self._app2, timeout=3)
    content = url.read()
    assert_true('application server' in content)

  def test_http_app_servers_big(self):
    '''Fetch big photo from app servers'''
    url = urllib2.urlopen('http://%s/big.jpg' % self._app1, timeout=3)
    content = url.read()
    assert_equal(len(content), self._big_photo_size)

    url = urllib2.urlopen('http://%s/big.jpg' % self._app2, timeout=3)
    content = url.read()
    assert_equal(len(content), self._big_photo_size)
