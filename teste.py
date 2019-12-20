#! /usr/bin/env python3

import subprocess as sp
import memory as m
import code
# import fire
import sys
import os


class MemorySegment(object):
    def __init__(self, base, size, perm, name):
        self.base = base
        self.size = size
        self.perm = perm
        self.name = name


def get_memory_mappings(pid):
    compl = sp.run(
        ["pmap", str(pid)],
        capture_output=True,
        check=True,
        text=True)

    for base, size, perm, name in (line.split(maxsplit=3) for line in compl.stdout.split(os.linesep)[1:-2]):
        yield MemorySegment(int(base, 16), int(size[:-1]) * 1024, perm, name)


if __name__ == "__main__":
    code.interact(local=locals())
