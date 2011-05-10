import os
import re
import sys
import socket
import subprocess


def send_cli_command(host, port, command, timeout=3):
  '''Send a cli command to a sphene router.

  Args:
    host: hostname of router
    port: tcp port for CLI
    command: string command to send
    timeout (optional): connection timeout in seconds

  Returns:
    True on success, False on failure (unable to connect to host)
  '''
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.settimeout(timeout)
  try:
    s.connect((host, port))
  except socket.error, e:
    return False

  s.send('\n')
  command = '%s\n' % command
  send_success = (s.send(command) == len(command))
  return send_success


def ping(host, count=1):
  '''Sends ICMP pings to a host.

  Args:
    host: target
    count (optional): number of pings to send

  Returns:
    True on 0% packet loss, False otherwise.
  '''
  ph = subprocess.Popen(['ping', '-c', str(count), host],
                        stdin=None,
                        stdout=subprocess.PIPE);
  output = ph.stdout.read()

  # Find packet loss.
  lines = output.split('\n')
  for line in lines:
    m = re.search('\s([0-9.]+)% packet loss', line)
    if m:
      loss = float(m.group(1))
      return (loss == 0)

  # Didn't find packet loss line?
  return False


def traceroute(host):
  '''Traceroutes a host.

  Args:
    host: target

  Returns:
    list of all hops in the path as strings, or None for an unknown hop
  '''
  ph = subprocess.Popen(['traceroute', host],
                        stdin=None,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
  output = ph.stdout.read()

  # Parse output lines.
  lines = output.split('\n')
  hops = []
  for line in lines:
    m = re.search('^\s*(\d+)\s+([a-z0-9\.\-\*]+)\s+', line, re.I)
    if m:
      dist = m.group(1)
      hop = m.group(2)
      if hop == '*':
        hop = None
      hops.append(hop)

  return hops
