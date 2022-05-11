#!/usr/bin/env python
from contextlib import contextmanager

@contextmanager
def func(arg):
    m = 1
    c = m
    try:
        print(c)
        yield 2
    finally:
        del m


if __name__ == "__main__":
    with func(1) as f:
        print(f)