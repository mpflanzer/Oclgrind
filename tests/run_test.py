# run_test.py (Oclgrind)
# Copyright (c) 2013-2016, James Price and Simon McIntosh-Smith,
# University of Bristol. All rights reserved.
#
# This program is provided under a three-clause BSD license. For full
# license terms please see the LICENSE file distributed with this
# source code.

import errno
import os
import re
import subprocess
import sys

# Check arguments
if len(sys.argv) != 3:
  print('Usage: python run_test.py OCLGRIND-KERNEL TEST_EXE|TEST.sim')
  sys.exit(1)
if not os.path.isfile(sys.argv[2]):
  print('Test file not found')
  sys.exit(1)

# Construct paths to test inputs/outputs
oclgrind_kernel = sys.argv[1]
test_full_path  = sys.argv[2]
test_dir        = os.path.dirname(os.path.realpath(test_full_path))
test_file       = os.path.basename(test_full_path)
test_name       = os.path.splitext(test_file)[0]
current_dir     = os.getcwd()

if test_file.endswith('.sim'):
  test_ref = test_full_path[:-4] + '.ref'
else:
  if test_full_path[0] == '/':
    rel_path = test_full_path[test_full_path.find('/tests/') + 7:]
  else:
    rel_path = test_full_path

  test_ref = os.path.dirname(os.path.abspath(__file__)) + os.path.sep \
    + rel_path + '.ref'

# Enable race detection and uninitialized memory plugins
os.environ["OCLGRIND_CHECK_API"] = "1"
os.environ["OCLGRIND_DATA_RACES"] = "1"
os.environ["OCLGRIND_UNINITIALIZED"] = "1"
os.environ["OCLGRIND_ARITHMETIC_EXCEPTIONS"] = "1"

# Enable all diagnostic output
os.environ["OCLGRIND_DIAGNOSTIC_OPTIONS"] = "-Wall"

def fail(ret=1):
  print('FAILED')
  sys.exit(ret)

def run(output_suffix):

  # Get filename for test output
  if test_file.endswith('.sim'):
    test_out = test_dir.split(os.path.sep)[-1] + os.path.sep + \
               test_name + output_suffix + '.out'
  else:
    test_out = test_dir + os.path.sep + \
               test_name + output_suffix + '.out'


  output_dir = os.path.dirname(test_out)
  try:
    os.makedirs(output_dir)
  except OSError as exc:
    if exc.errno == errno.EEXIST and os.path.isdir(output_dir):
      pass
    else:
      raise

  out = open(test_out, 'w')

  # Run test
  if test_file.endswith('.sim'):
    os.chdir(test_dir)
    retval = subprocess.call([oclgrind_kernel, test_file],
                             stdout=out, stderr=out)
    os.chdir(current_dir)
  else:
    retval = subprocess.call([test_full_path], stdout=out, stderr=out)

  out.close()
  if retval != 0:
    print('Test returned non-zero value (' + str(retval) + ')')
    fail(retval)

  # Compare output to reference file (if provided)
  if os.path.isfile(test_ref):

    # Open output and reference files
    out = open(test_out).read().splitlines()
    ref = open(test_ref).read().splitlines()

    # Check output matches references
    oi = 0
    for line in ref:
      if len(line) == 0:
        continue

      type = line.split()[0]
      text = line[6:]

      # Find next non-blank line in output file
      while not len(out[oi]):
        oi += 1

      if type == 'ERROR':
        # Check first line of error contains reference message
        if not text in out[oi]:
          print('Expected '  + line)
          print('Found    "' + out[oi] + '"')
          fail()
        # Skip remaining lines of error
        while oi < len(out) and len(out[oi]):
          oi += 1
      elif type == 'EXACT':
        # Check line of output matches reference exactly
        if not text == out[oi]:
          print('Expected '  + line)
          print('Found    "' + out[oi] + '"')
          fail()
        oi += 1
      elif type == 'MATCH':
        # Check line of output contains reference text
        if not text in out[oi]:
          print('Expected '  + line)
          print('Found    "' + out[oi] + '"')
          fail()
        oi += 1
      else:
        print('Invalid match type in reference file')
        fail()

print('Running test with optimisations')
run('')
print('PASSED')

print('')
print('Running test without optimisations')
os.environ["OCLGRIND_BUILD_OPTIONS"] = "-cl-opt-disable"
run('_noopt')
print('PASSED')

# Test passed
sys.exit(0)
