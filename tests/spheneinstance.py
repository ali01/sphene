import getpass
import os
import subprocess


class SpheneInstance(object):
  def __init__(self, topo_id, cli_port, auth_key_file,
               rtable_file=None, vhost=None):
    tests_dir = os.path.dirname(os.path.abspath(__file__))
    base_dir = os.path.normpath(os.path.join(tests_dir, '..'))
    binary = os.path.normpath(os.path.join(base_dir, 'build', binary_name)

    cmd = [binary,
           '-a', auth_key_file,
           '-t', str(topo_id),
           '-s', 'vns-1.stanford.edu',
           '-u', getpass.getuser(),
           '-c', str(cli_port)]

    if rtable_file:
      cmd.extend(('-r', rtable_file))
    if vhost:
      cmd.extend(('-v', vhost))
    if os.getenv('DEBUG', 0):
      cmd.append('-d')

    self._cli_port = cli_port
    self._instance = subprocess.Popen(cmd)

  def __del__(self):
    if self._instance:
      self._instance.terminate()

  def cli_port(self):
    return self._cli_port
