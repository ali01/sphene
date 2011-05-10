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


def base_dir():
  '''Returns the base path of the sphene project directory.'''
  return os.path.normpath(os.path.join(tests_dir(), '..'))


def tests_dir():
  '''Returns the path to the tests directory that contains this file.'''
  return os.path.dirname(os.path.abspath(__file__))


def sphene_binary():
  '''Returns the path to the built sphene binary.'''
  return os.path.normpath(os.path.join(base_dir(), 'build', 'sr'))


class Test_Topo594:
  def setUp(self):
    self._instance = None

    self._topo_id = 594
    self._big_photo_size = 1053791  # bytes
    self._cli_port = 2300
    self._rtr_eth0 = '10.3.0.24'
    self._rtr_eth1 = '10.3.0.28'
    self._rtr_eth2 = '10.3.0.30'
    self._app1 = '10.3.0.29'
    self._app2 = '10.3.0.31'

    # Should we bootstrap the system with our own sphene instance?
    if os.getenv('BOOTSTRAP', 1):
      self._cli_port = 23000  # we can set our own when bootstrapping
      self._bootstrap()

  def tearDown(self):
    if self._instance:
      self._instance.terminate()

  def _bootstrap(self):
    '''Initialize a sphene instance for tests.'''
    auth_key_file = os.path.join(base_dir(), 'auth_key')
    rtable_file = os.path.join(base_dir(), 'rtable_%d' % self._topo_id)
    cmd = [sphene_binary(),
           '-a', auth_key_file,
           '-t', str(self._topo_id),
           '-r', rtable_file,
           '-s', 'vns-1.stanford.edu',
           '-u', getpass.getuser(),
           '-c', str(self._cli_port)]
    # Enable debug mode through an environment variable.
    if os.getenv('DEBUG', 0):
      cmd.append('-d')
    self._instance = subprocess.Popen(cmd)

    # Wait for interface to come up.
    for i in range(5):
      if network_lib.ping(self._rtr_eth0):
        return

    # Failed to bootstrap.
    raise Exception('timeout waiting for router to come up')

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
    url = urllib2.urlopen('http://%s' % self._app1, timeout=3)
    content = url.read()
    assert_true('application server' in content)

  def test_http_app_server2_basic(self):
    '''Test basic HTTP on app server 2'''
    url = urllib2.urlopen('http://%s' % self._app2, timeout=3)
    content = url.read()
    assert_true('application server' in content)

  def test_http_app_server1_big(self):
    '''Fetch big photo from app server 1'''
    url = urllib2.urlopen('http://%s/big.jpg' % self._app1, timeout=3)
    content = url.read()
    assert_equal(len(content), self._big_photo_size)

  def test_http_app_server2_big(self):
    '''Fetch big photo from app server 2'''
    url = urllib2.urlopen('http://%s/big.jpg' % self._app2, timeout=3)
    content = url.read()
    assert_equal(len(content), self._big_photo_size)

  def test_break_link_app_server1(self):
    '''Break link to app server 1 and ping'''
    assert_true(network_lib.send_cli_command(
        self._rtr_eth0, self._cli_port, 'ip intf eth1 down'))
    assert_true(network_lib.ping(self._rtr_eth0))
    assert_false(network_lib.ping(self._app1))
    assert_true(network_lib.ping(self._app2))

    assert_true(network_lib.send_cli_command(
        self._rtr_eth0, self._cli_port, 'ip intf eth1 up'))
    assert_true(network_lib.ping(self._rtr_eth0))
    assert_true(network_lib.ping(self._app1))
    assert_true(network_lib.ping(self._app2))

  def test_break_link_app_server2(self):
    '''Break link to app server 2 and ping'''
    assert_true(network_lib.send_cli_command(
        self._rtr_eth0, self._cli_port, 'ip intf eth2 down'))
    assert_true(network_lib.ping(self._rtr_eth0))
    assert_true(network_lib.ping(self._app1))
    assert_false(network_lib.ping(self._app2))

    assert_true(network_lib.send_cli_command(
        self._rtr_eth0, self._cli_port, 'ip intf eth2 up'))
    assert_true(network_lib.ping(self._rtr_eth0))
    assert_true(network_lib.ping(self._app1))
    assert_true(network_lib.ping(self._app2))
