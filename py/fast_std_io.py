import os

def os_read(size_to_read: int) -> bytes:
  buf = b''
  while len(buf) < size_to_read:
    buf += os.read(0, size_to_read - len(buf))
  return buf

def os_write(data: bytes) -> None:
  os.write(1, data)

def os_writeline(data: str) -> None:
  os.write(1, (data + '\n').encode())


import sys

def sys_read(size_to_read: int) -> bytes:
  buf = b''
  while len(buf) < size_to_read:
    buf += sys.stdin.buffer.read(size_to_read - len(buf))
  return buf

def sys_write(data: bytes) -> None:
  sys.stdout.buffer.write(data)
  sys.stdout.buffer.flush()

def sys_writeline(data: str) -> None:
  sys.stdout.write(data + '\n')
  sys.stdout.flush()