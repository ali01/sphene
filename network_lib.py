import os
import re
import sys
import socket
import subprocess


def send_cli_command(host, port, command):
  '''Send a cli command to a sphene router.

  Args:
    host: hostname of router
    port: tcp port for CLI
    command: string command to send

  Returns:
    True on success, False on failure (unable to connect to host)
  '''
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  try:
    s.connect((host, port))
  except socket.error, e:
    return False

  s.send('\n')
  command = '%s\n' % command
  send_success = (s.send(command) == len(command))
  return send_success


def ping(host):
  '''Sends 3 ICMP pings to a host.

  Args:
    host: target

  Returns:
    True on 0% packet loss, False otherwise.
  '''
  ph = subprocess.Popen(['ping', '-c', '3', host],
                        stdin=None,
                        stdout=subprocess.PIPE);
  output = ph.stdout.read()

  # Find packet loss.
  lines = output.split('\n')
  for line in lines:
    m = re.search('([0-9.])% packet loss', line)
    if m:
      loss = float(m.group(1))
      print loss
      return (loss == 0)

  # Didn't find packet loss line?
  return False
