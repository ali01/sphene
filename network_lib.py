import os
import sys
import socket


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
