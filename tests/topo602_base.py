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

    # Destruct existing instances (if any).
    instance1 = None
    instance2 = None
    instance3 = None

    cli_port = 23000
    instance1 = self._bootstrap(self._binary1, cli_port, 'vhost1',
                                pass_rtable=True)
    instance2 = self._bootstrap(self._binary2, cli_port + 1, 'vhost2',
                                pass_rtable=False)
    instance3 = self._bootstrap(self._binary3, cli_port + 2, 'vhost3',
                                pass_rtable=False)

    # Wait for convergence.
    time.sleep(13)  # a few seconds after a full helloint of 10 seconds

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
