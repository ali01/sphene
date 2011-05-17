'''
Run a live test against a Sphene router running in VNS with topology 594.

                                  eth0:10.3.0.115
                                  +======================+
                                  |  router #2         (eth1) ======= App
                                  Server 1 (10.3.0.119)
                                  |  vhost2              | eth1:10.3.0.118
                                  +=====(eth0)=====(eth2)+
                                          /          ||    eth2:10.3.0.120
                                         /           ||
                                        /            ||
                                       /             ||
                                      /              ||  vhost1:
  (internet)           +============(eth1)==+        ||    eth0: 10.3.0.113
  gateway rtr ======= (eth0)  router #1     |        ||    eth1: 10.3.0.114
  172.24.74.17         |      vhost1        |        ||    eth2: 10.3.0.116
                       +============(eth2)==+        ||
                                      \              ||
                                       \             ||
                                        \            ||
                                         \           ||
                                          \          ||    eth2:10.3.0.121
                                  +=====(eth0)=====(eth2)+
                                  |  router #3           | eth1:10.3.0.122
                                  |  vhost3            (eth1) ======= App
                                  Server 2 (10.3.0.123)
                                  +======================+
                                  eth0:10.3.0.117
'''
from nose.tools import *
import os
import sys
import time
import urllib2

import network_lib
import spheneinstance

# Global instances to avoid setup and teardown for each test.
instance1 = None
instance2 = None
instance3 = None


def base_dir():
  '''Returns the base path of the sphene project directory.'''
  tests_dir = os.path.dirname(os.path.abspath(__file__))
  return os.path.relpath(os.path.join(tests_dir, '..'))


BINARY = os.getenv('BINARY', os.path.join(base_dir(), 'build', 'sr'))
REF_BINARY = os.getenv('REF_BINARY', os.path.join(base_dir(), 'sr_ref'))

helloint = 13


class Topo602:
  def __init__(self, binary1, binary2, binary3):
    '''Base class for topology 602 tests.'''
    self._binary1 = binary1
    self._binary2 = binary2
    self._binary3 = binary3

  def setUp(self):
    self._topo_id = 602
    self._big_photo_size = 1053791  # bytes
    self._rtr1_eth0 = '10.3.0.113'
    self._rtr1_eth1 = '10.3.0.114'
    self._rtr1_eth2 = '10.3.0.116'
    self._rtr2_eth0 = '10.3.0.115'
    self._rtr2_eth1 = '10.3.0.118'
    self._rtr2_eth2 = '10.3.0.120'
    self._rtr3_eth0 = '10.3.0.117'
    self._rtr3_eth1 = '10.3.0.122'
    self._rtr3_eth2 = '10.3.0.121'
    self._app1 = '10.3.0.119'
    self._app2 = '10.3.0.123'

    # Setup instances if they don't exist yet.
    if not instance1 or not instance2 or not instance3:
      self._reset()

  def _reset(self):
    '''Re-bootstrap.'''
    global instance1
    global instance2
    global instance3

    # Kill existing instances (if any).
    if instance1:
      instance1.stop()
    if instance2:
      instance2.stop()
    if instance3:
      instance3.stop()

    cli_port = 23000
    instance1 = self._bootstrap(self._binary1, cli_port, 'vhost1',
                                pass_rtable=True)
    instance1.start()
    instance2 = self._bootstrap(self._binary2, cli_port + 1, 'vhost2',
                                pass_rtable=False)
    instance2.start()
    instance3 = self._bootstrap(self._binary3, cli_port + 2, 'vhost3',
                                pass_rtable=False)
    instance3.start()
    print 'bootstrapping complete.'

    # Wait for convergence.
    time.sleep(2 * helloint)  # including VNS startup time
    print 'convergence time passed.'

  def _bootstrap(self, binary, cli_port, vhost, pass_rtable=False):
    '''Initialize a sphene instance for tests.'''
    auth_key_file = os.path.join(base_dir(), 'auth_key')
    if pass_rtable:
      rtable_file = os.path.join(base_dir(), 'rtable_%d' % self._topo_id)
    else:
      rtable_file = os.path.join(base_dir(), 'rtable_empty')
    return spheneinstance.SpheneInstance(self._topo_id, cli_port, auth_key_file,
                                         rtable_file=rtable_file, vhost=vhost,
                                         binary=binary)

  def test_ping_router1_interfaces(self):
    '''Ping router 1 interfaces'''
    assert_true(network_lib.ping(self._rtr1_eth0))
    assert_true(network_lib.ping(self._rtr1_eth1))
    assert_true(network_lib.ping(self._rtr1_eth2))

  def test_ping_router2_interfaces(self):
    '''Ping router 2 interfaces'''
    assert_true(network_lib.ping(self._rtr2_eth0))
    assert_true(network_lib.ping(self._rtr2_eth1))
    assert_true(network_lib.ping(self._rtr2_eth2))

  def test_ping_router3_interfaces(self):
    '''Ping router 3 interfaces'''
    assert_true(network_lib.ping(self._rtr3_eth0))
    assert_true(network_lib.ping(self._rtr3_eth1))
    assert_true(network_lib.ping(self._rtr3_eth2))

  def test_ping_app_servers(self):
    '''Ping app servers'''
    assert_true(network_lib.ping(self._app1))
    assert_true(network_lib.ping(self._app2))

  def _disable_interface(self, instance, iface_name):
    '''Disable an interface on a router.'''
    host = 'localhost'
    port = instance.cli_port()

    # Turn off interface.
    cmd = 'ip intf %s down' % iface_name
    assert_true(network_lib.send_cli_command(host, port, cmd))

  def _enable_interface(self, instance, iface_name):
    '''Enable an interface on a router.'''
    host = 'localhost'
    port = instance.cli_port()

    # Turn on interface.
    cmd = 'ip intf %s up' % iface_name
    assert_true(network_lib.send_cli_command(host, port, cmd))

  def _basic_connectivity_tests(self):
    self.test_ping_router1_interfaces()
    self.test_ping_router2_interfaces()
    self.test_ping_router3_interfaces()
    self.test_ping_app_servers()

  def test_disable_rtr1_eth1(self):
    '''Disable router 1 eth1'''
    self._disable_interface(instance1, 'eth1')
    time.sleep(3 * helloint)
    self._basic_connectivity_tests()
    assert_equal(
        network_lib.traceroute(self._app1)[-4:],
        [self._rtr1_eth0, self._rtr3_eth0, self._rtr2_eth2, self._app1])

    self._enable_interface(instance1, 'eth1')
    time.sleep(3 * helloint)
    self._basic_connectivity_tests()
    assert_equal(
        network_lib.traceroute(self._app1)[-3:],
        [self._rtr1_eth0, self._rtr2_eth0, self._app1])


def kill_all_instances():
  if instance1:
    instance1.stop()
  if instance2:
    instance2.stop()
  if instance3:
    instance3.stop()
