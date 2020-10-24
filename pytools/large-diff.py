#!/usr/bin/env python3
#coding: utf-8

"""If two very large files(e.g. 1GB+) looks mostly identical but are actually different.
How do you find out where the different bytes start?

This program helps out, it compares both files chunk by chunk. Each block
defaults to 1M bytes, and write different blocks as separate files for easier analysis.

"""

import os, sys

