'''
Run a live test against a Sphene router running in VNS with topology 677.
'''
from nose.tools import *
import os
import sys
import time
import urllib2

import network_lib
import spheneinstance

# Global instances to avoid setup and teardown for each test.
instance0 = None
instance1 = None
instance2 = None
instance3 = None
instance4 = None
instance5 = None


def base_dir():
  '''Returns the base path of the sphene project directory.'''
  tests_dir = os.path.dirname(os.path.abspath(__file__))
  return os.path.relpath(os.path.join(tests_dir, '..'))


BINARY = os.getenv('BINARY', os.path.join(base_dir(), 'build', 'sr'))
REF_BINARY = os.getenv('REF_BINARY', os.path.join(base_dir(), 'sr_ref'))

helloint = 5


class Router(object):
  def __init__(self, id, eth0, eth1, eth2, eth3):
    self.id = id
    self.eth0 = eth0
    self.eth1 = eth1
    self.eth2 = eth2
    self.eth3 = eth3


class Test_Topo677:
  def __init__(self):
    '''Test for topology 677.'''
    self._binary0 = os.getenv('BINARY0', BINARY)
    self._binary1 = os.getenv('BINARY1', BINARY)
    self._binary2 = os.getenv('BINARY2', BINARY)
    self._binary3 = os.getenv('BINARY3', BINARY)
    self._binary4 = os.getenv('BINARY4', BINARY)
    self._binary5 = os.getenv('BINARY5', BINARY)

  def setUp(self):
    self._topo_id = 677
    self._big_photo_size = 1053791  # bytes

    self._rtr0 = Router(0, '10.3.1.65', '10.3.1.78', '10.3.1.80', '10.3.1.66')
    self._rtr1 = Router(1, '10.3.1.90', '10.3.1.81', '10.3.1.82', '10.3.1.68')
    self._rtr2 = Router(2, '10.3.1.98', '10.3.1.83', '10.3.1.84', '10.3.1.70')
    self._rtr3 = Router(3, '10.3.1.92', '10.3.1.85', '10.3.1.86', '10.3.1.72')
    self._rtr4 = Router(4, '10.3.1.94', '10.3.1.87', '10.3.1.88', '10.3.1.74')
    self._rtr5 = Router(5, '10.3.1.96', '10.3.1.89', '10.3.1.79', '10.3.1.76')

    self._app1 = '10.3.1.91'
    self._app2 = '10.3.1.99'
    self._app3 = '10.3.1.93'
    self._app4 = '10.3.1.95'
    self._app5 = '10.3.1.97'

    # Setup instances if they don't exist yet.
    if (not instance0 or not instance1 or not instance2 or
        not instance3 or not instance4 or not instance5):
      self._reset()

  def _reset(self):
    '''Re-bootstrap.'''
    global instance0
    global instance1
    global instance2
    global instance3
    global instance4
    global instance5

    # Kill existing instances (if any).
    if instance0:
      instance0.stop()
    if instance1:
      instance1.stop()
    if instance2:
      instance2.stop()
    if instance3:
      instance3.stop()
    if instance4:
      instance4.stop()
    if instance5:
      instance5.stop()

    cli_port = 23000

    instance0 = self._bootstrap(self._binary0, cli_port, 'Router0',
                                pass_rtable=True)
    instance0.start()
    print 'router 0 up.'

    instance1 = self._bootstrap(self._binary1, cli_port + 1, 'Router1',
                                pass_rtable=False)
    instance1.start()
    print 'router 1 up.'

    instance2 = self._bootstrap(self._binary2, cli_port + 2, 'Router2',
                                pass_rtable=False)
    instance2.start()
    print 'router 2 up.'

    instance3 = self._bootstrap(self._binary3, cli_port + 3, 'Router3',
                                pass_rtable=False)
    instance3.start()
    print 'router 3 up.'

    instance4 = self._bootstrap(self._binary4, cli_port + 4, 'Router4',
                                pass_rtable=False)
    instance4.start()
    print 'router 4 up.'

    instance5 = self._bootstrap(self._binary5, cli_port + 5, 'Router5',
                                pass_rtable=False)
    instance5.start()
    print 'router 5 up.'

    print 'bootstrapping complete.'

    # Wait for convergence.
    time.sleep(2 * helloint)  # including VNS startup time
    print 'convergence time passed.'

  def _bootstrap(self, binary, cli_port, vhost, pass_rtable=False):
    '''Initialize a sphene instance for tests.'''
    auth_key_file = os.path.join(base_dir(), 'auth_key.eastzone')
    if pass_rtable:
      rtable_file = os.path.join(base_dir(), 'rtable_%d' % self._topo_id)
    else:
      rtable_file = os.path.join(base_dir(), 'rtable_empty')
    return spheneinstance.SpheneInstance(self._topo_id, cli_port, auth_key_file,
                                         rtable_file=rtable_file, vhost=vhost,
                                         binary=binary, username='eastzone')

  def _ping_router_interfaces(self, router):
    assert_true(network_lib.ping(router.eth0))
    assert_true(network_lib.ping(router.eth1))
    assert_true(network_lib.ping(router.eth2))
    assert_true(network_lib.ping(router.eth3))

  def test_ping_router0_interfaces(self):
    '''Ping router 0 interfaces'''
    self._ping_router_interfaces(self._rtr0)

  def test_ping_router1_interfaces(self):
    '''Ping router 1 interfaces'''
    self._ping_router_interfaces(self._rtr1)

  def test_ping_router2_interfaces(self):
    '''Ping router 2 interfaces'''
    self._ping_router_interfaces(self._rtr2)

  def test_ping_router3_interfaces(self):
    '''Ping router 3 interfaces'''
    self._ping_router_interfaces(self._rtr3)

  def test_ping_router4_interfaces(self):
    '''Ping router 4 interfaces'''
    self._ping_router_interfaces(self._rtr4)

  def test_ping_router5_interfaces(self):
    '''Ping router 5 interfaces'''
    self._ping_router_interfaces(self._rtr5)

  def test_ping_app_servers(self):
    '''Ping app servers'''
    assert_true(network_lib.ping(self._app1))
    assert_true(network_lib.ping(self._app2))
    assert_true(network_lib.ping(self._app3))
    assert_true(network_lib.ping(self._app4))
    assert_true(network_lib.ping(self._app5))

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
    self.test_ping_router0_interfaces()
    self.test_ping_router1_interfaces()
    self.test_ping_router2_interfaces()
    self.test_ping_router3_interfaces()
    self.test_ping_router4_interfaces()
    self.test_ping_router5_interfaces()
    self.test_ping_app_servers()

  def test_disable_0_internal_links_cut(self):
    '''Cut router 0 eth1 and eth2, access S1'''
    self._disable_interface(instance0, 'eth1')
    self._disable_interface(instance0, 'eth2')
    conv_time = (3 * (0 + 1) + 1) * helloint
    time.sleep(conv_time)
    self._basic_connectivity_tests()
    assert_equal(
        network_lib.traceroute(self._app1)[-3:],
        [self._rtr0.eth0, self._rtr1.eth3, self._app1])

    self._enable_interface(instance0, 'eth1')
    self._enable_interface(instance0, 'eth2')
    time.sleep(2 * helloint)
    self._basic_connectivity_tests()

  def test_disable_1_internal_link_cut(self):
    '''Cut router 0 eth1 and eth2 + 1 internal link, access S1'''
    self._disable_interface(instance0, 'eth1')
    self._disable_interface(instance0, 'eth2')
    self._disable_interface(instance1, 'eth3')
    conv_time = (3 * (1 + 1) + 1) * helloint
    time.sleep(conv_time)
    self.test_ping_app_servers()
    assert_equal(
        network_lib.traceroute(self._app1)[-4:],
        [self._rtr0.eth0, self._rtr2.eth3, self._rtr1.eth2, self._app1])

    self._enable_interface(instance0, 'eth1')
    self._enable_interface(instance0, 'eth2')
    self._enable_interface(instance1, 'eth3')
    time.sleep(2 * helloint)
    self._basic_connectivity_tests()

  def test_disable_2_internal_links_cut(self):
    '''Cut router 0 eth1 and eth2 + 2 internal links, access S1'''
    self._disable_interface(instance0, 'eth1')
    self._disable_interface(instance0, 'eth2')
    self._disable_interface(instance1, 'eth3')
    self._disable_interface(instance2, 'eth3')
    conv_time = (3 * (2 + 1) + 1) * helloint
    time.sleep(conv_time)
    self.test_ping_app_servers()
    assert_equal(
        network_lib.traceroute(self._app1)[-5:],
        [self._rtr0.eth0, self._rtr3.eth3, self._rtr2.eth2, self._rtr1.eth2,
         self._app1])

    self._enable_interface(instance0, 'eth1')
    self._enable_interface(instance0, 'eth2')
    self._enable_interface(instance1, 'eth3')
    self._enable_interface(instance2, 'eth3')
    time.sleep(2 * helloint)
    self._basic_connectivity_tests()

  def test_disable_3_internal_links_cut(self):
    '''Cut router 0 eth1 and eth2 + 3 internal links, access S1'''
    self._disable_interface(instance0, 'eth1')
    self._disable_interface(instance0, 'eth2')
    self._disable_interface(instance1, 'eth3')
    self._disable_interface(instance2, 'eth3')
    self._disable_interface(instance3, 'eth3')
    conv_time = (3 * (3 + 1) + 1) * helloint
    time.sleep(conv_time)
    self.test_ping_app_servers()
    assert_equal(
        network_lib.traceroute(self._app1)[-6:],
        [self._rtr0.eth0, self._rtr4.eth3, self._rtr3.eth2, self._rtr2.eth2,
         self._rtr1.eth2, self._app1])

    self._enable_interface(instance0, 'eth1')
    self._enable_interface(instance0, 'eth2')
    self._enable_interface(instance1, 'eth3')
    self._enable_interface(instance2, 'eth3')
    self._enable_interface(instance3, 'eth3')
    time.sleep(2 * helloint)
    self._basic_connectivity_tests()

  def test_disable_4_internal_links_cut(self):
    '''Cut router 0 eth1 and eth2 + 4 internal links, access S1'''
    self._disable_interface(instance0, 'eth1')
    self._disable_interface(instance0, 'eth2')
    self._disable_interface(instance1, 'eth3')
    self._disable_interface(instance2, 'eth3')
    self._disable_interface(instance3, 'eth3')
    self._disable_interface(instance4, 'eth3')
    conv_time = (3 * (4 + 1) + 1) * helloint
    time.sleep(conv_time)
    self.test_ping_app_servers()
    assert_equal(
        network_lib.traceroute(self._app1)[-7:],
        [self._rtr0.eth0, self._rtr5.eth3, self._rtr4.eth2, self._rtr3.eth2,
         self._rtr2.eth2, self._rtr1.eth2, self._app1])

    self._enable_interface(instance0, 'eth1')
    self._enable_interface(instance0, 'eth2')
    self._enable_interface(instance1, 'eth3')
    self._enable_interface(instance2, 'eth3')
    self._enable_interface(instance3, 'eth3')
    self._enable_interface(instance4, 'eth3')
    time.sleep(2 * helloint)
    self._basic_connectivity_tests()

  def test_disable_5_internal_links_cut(self):
    '''Cut router 0 eth1 and eth2 + 5 internal links, access S1'''
    self._disable_interface(instance0, 'eth1')
    self._disable_interface(instance0, 'eth2')
    self._disable_interface(instance1, 'eth3')
    self._disable_interface(instance2, 'eth3')
    self._disable_interface(instance3, 'eth3')
    self._disable_interface(instance4, 'eth3')
    self._disable_interface(instance5, 'eth3')
    time.sleep(4 * helloint)
    assert_false(network_lib.ping(self._app1))

    self._enable_interface(instance0, 'eth1')
    self._enable_interface(instance0, 'eth2')
    self._enable_interface(instance1, 'eth3')
    self._enable_interface(instance2, 'eth3')
    self._enable_interface(instance3, 'eth3')
    self._enable_interface(instance4, 'eth3')
    self._enable_interface(instance5, 'eth3')
    time.sleep(2 * helloint)
    self._basic_connectivity_tests()


def teardown():
  if instance0:
    instance0.stop()
  if instance1:
    instance1.stop()
  if instance2:
    instance2.stop()
  if instance3:
    instance3.stop()
  if instance4:
    instance4.stop()
  if instance5:
    instance5.stop()
  time.sleep(3)
