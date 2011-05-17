import fcntl
import getpass
import os
import select
import subprocess
import sys
import threading


class SpheneInstance(object):
  def __init__(self, topo_id, cli_port, auth_key_file,
               rtable_file=None, vhost=None, binary=None):
    tests_dir = os.path.dirname(os.path.abspath(__file__))
    base_dir = os.path.normpath(os.path.join(tests_dir, '..'))

    if binary is None:
      binary_name = 'sr'
      binary = os.path.normpath(os.path.join(base_dir, 'build', binary_name))

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

    self._binary = binary
    self._cli_port = cli_port
    self._cmd = cmd
    self._instance = None
    self._quit = False
    self._thread_stdout = None
    self._thread_stderr = None

  def __del__(self):
    self.stop()

  def _stream_reader(self, in_stream, out_stream):
    fd = in_stream.fileno()
    fl = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)

    p = select.poll()
    p.register(fd, select.POLLIN)

    while not self._quit:
      events = p.poll(1000)
      if not events:
        continue
      buf = in_stream.read()
      if not buf:  # EOF
        return
      if buf and os.getenv('DEBUG', 0):
        out_stream.write(buf)

  def start(self):
    self._quit = False
    self._instance = subprocess.Popen(self._cmd,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)

    # Threads to read stdout and stderr.
    self._thread_stdout = threading.Thread(target=self._stream_reader,
                                           args=(self._instance.stdout,
                                                 sys.stdout))
    self._thread_stdout.daemon = True
    self._thread_stdout.start()

    self._thread_stderr = threading.Thread(target=self._stream_reader,
                                           args=(self._instance.stderr,
                                                 sys.stdout))
    self._thread_stderr.daemon = True
    self._thread_stderr.start()

  def stop(self):
    self._quit = True
    if self._thread_stdout and self._thread_stdout.is_alive():
      self._thread_stdout.join()
    if self._thread_stderr and self._thread_stderr.is_alive():
      self._thread_stderr.join()
    if self._instance:
      self._instance.terminate()

  def cli_port(self):
    return self._cli_port
