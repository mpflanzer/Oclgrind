// main.cpp (Oclgrind)
// Copyright (c) 2013-2015, James Price and Simon McIntosh-Smith,
// University of Bristol. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#include "config.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "kernel/Simulation.h"

using namespace oclgrind;
using namespace std;

static bool outputGlobalMemory = false;
static const char *simfile = NULL;

static bool parseArguments(int argc, char *argv[]);
static void printUsage();
static void setEnvironment(const char *name, const char *value);

int main(int argc, char *argv[])
{
  // Parse arguments
  if (!parseArguments(argc, argv))
  {
    return 1;
  }

  // Initialise simulation
  Simulation simulation;
  if (!simulation.load(simfile))
  {
    return 1;
  }

  // Run simulation
  simulation.run(outputGlobalMemory);
}

static bool parseArguments(int argc, char *argv[])
{
  // Collect all diagnostic options before writing them to environment
  std::vector<const char*> diagnosticOptions;

  for (int i = 1; i < argc; i++)
  {
    if (!strncmp(argv[i], "-W", 2))
    {
      diagnosticOptions.push_back(argv[i]);
    }
    else if (!strcmp(argv[i], "--build-options"))
    {
      if (++i >= argc)
      {
        cerr << "Missing argument to --build-options" << endl;
        return false;
      }
      setEnvironment("OCLGRIND_BUILD_OPTIONS", argv[i]);
    }
    else if (!strcmp(argv[i], "--data-races"))
    {
      setEnvironment("OCLGRIND_DATA_RACES", "1");
    }
    else if (!strcmp(argv[i], "--disable-pch"))
    {
      setEnvironment("OCLGRIND_DISABLE_PCH", "1");
    }
    else if (!strcmp(argv[i], "--dump-spir"))
    {
      setEnvironment("OCLGRIND_DUMP_SPIR", "1");
    }
    else if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "--global-mem"))
    {
      outputGlobalMemory = true;
    }
    else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
    {
      printUsage();
      exit(0);
    }
    else if (!strcmp(argv[i], "--inst-counts"))
    {
      setEnvironment("OCLGRIND_INST_COUNTS", "1");
    }
    else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--interactive"))
    {
      setEnvironment("OCLGRIND_INTERACTIVE", "1");
    }
    else if (!strcmp(argv[i], "--log"))
    {
      if (++i >= argc)
      {
        cerr << "Missing argument to --log" << endl;
        return false;
      }
      setEnvironment("OCLGRIND_LOG", argv[i]);
    }
    else if (!strcmp(argv[i], "--max-errors"))
    {
      if (++i >= argc)
      {
        cerr << "Missing argument to --max-errors" << endl;
        return false;
      }
      setEnvironment("OCLGRIND_MAX_ERRORS", argv[i]);
    }
    else if (!strcmp(argv[i], "--num-threads"))
    {
      if (++i >= argc)
      {
        cerr << "Missing argument to --num-threads" << endl;
        return false;
      }
      setEnvironment("OCLGRIND_NUM_THREADS", argv[i]);
    }
    else if (!strcmp(argv[i], "--pch-dir"))
    {
      if (++i >= argc)
      {
        cerr << "Missing argument to --pch-dir" << endl;
        return false;
      }
      setEnvironment("OCLGRIND_PCH_DIR", argv[i]);
    }
    else if (!strcmp(argv[i], "--plugins"))
    {
      if (++i >= argc)
      {
        cerr << "Missing argument to --plugins" << endl;
        return false;
      }
      setEnvironment("OCLGRIND_PLUGINS", argv[i]);
    }
    else if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quick"))
    {
      setEnvironment("OCLGRIND_QUICK", "1");
    }
    else if (!strcmp(argv[i], "--stop-errors"))
    {
      if (++i >= argc)
      {
        cerr << "Missing argument to --stop-errors" << endl;
        return false;
      }
      setEnvironment("OCLGRIND_STOP_ERRORS", argv[i]);
    }
    else if (!strcmp(argv[i], "--uniform-writes"))
    {
      setEnvironment("OCLGRIND_UNIFORM_WRITES", "1");
    }
    else if (!strcmp(argv[i], "--uninitialized"))
    {
      setEnvironment("OCLGRIND_UNINITIALIZED", "1");
    }
    else if (!strcmp(argv[i], "--memcheck-uninitialized"))
    {
      setEnvironment("OCLGRIND_MEMCHECK_UNINITIALIZED", "1");
    }
    else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
    {
      cout << endl;
      cout << "Oclgrind " PACKAGE_VERSION << endl;
      cout << endl;
      cout << "Copyright (c) 2013-2015" << endl;
      cout << "James Price and Simon McIntosh-Smith, University of Bristol"
           << endl;
      cout << "https://github.com/jrprice/Oclgrind" << endl;
      cout << endl;
      exit(0);
    }
    else if (argv[i][0] == '-')
    {
      cerr << "Unrecognised option '" << argv[i] << "'" << endl;
      return false;
    }
    else
    {
      if (simfile == NULL)
      {
        simfile = argv[i];
      }
      else
      {
        cerr << "Unexpected positional argument '" << argv[i] << "'" << endl;
        return false;
      }
    }
  }

  // Set diagnostic options
  if(diagnosticOptions.size())
  {
      std::stringstream options;
      options << diagnosticOptions.front();

      for(int i = 1; i < diagnosticOptions.size(); ++i)
      {
        options << " " << diagnosticOptions[i];
      }

      setEnvironment("OCLGRIND_DIAGNOSTIC_OPTIONS", options.str().c_str());
  }

  if (simfile == NULL)
  {
    printUsage();
    return false;
  }

  return true;
}

static void printUsage()
{
  cout
    << "Usage: oclgrind-kernel [OPTIONS] simfile" << endl
    << "       oclgrind-kernel [--help | --version]" << endl
    << endl
    << "Options:" << endl
    << "     --build-options  OPTIONS  "
             "Additional options to pass to the OpenCL compiler" << endl
    << "     --data-races              "
             "Enable data-race detection" << endl
    << "     --disable-pch             "
             "Don't use precompiled headers" << endl
    << "     --dump-spir               "
             "Dump SPIR to /tmp/oclgrind_*.{ll,bc}" << endl
    << "  -g --global-mem              "
             "Output global memory at exit" << endl
    << "  -h --help                    "
             "Display usage information" << endl
    << "     --inst-counts             "
             "Output histograms of instructions executed" << endl
    << "  -i --interactive             "
             "Enable interactive mode" << endl
    << "     --log            LOGFILE  "
             "Redirect log/error messages to a file" << endl
    << "     --max-errors     NUM      "
             "Limit the number of error/warning messages" << endl
    << "  -Wall                        "
             "Enable all error/warning messages" << endl
    << "  -W[no-]MSG_GROUP             "
             "Enable/disable specific message group" << endl
    << "     --memcheck-uninitialized  "
             "Report usage of uninitialized values" << endl
    << "     --num-threads    NUM      "
             "Set the number of worker threads to use" << endl
    << "     --pch-dir        DIR      "
             "Override directory containing precompiled headers" << endl
    << "     --plugins        PLUGINS  "
             "Load colon separated list of plugin libraries" << endl
    << "  -q --quick                   "
             "Only run first and last work-group" << endl
    << "     --stop-errors    NUM      "
             "Abort the execution after NUM error/warning messages" << endl
    << "     --uniform-writes          "
             "Don't suppress uniform write-write data-races" << endl
    << "     --uninitialized           "
             "Report loads from uninitialized memory locations" << endl
    << "  -v --version                 "
             "Display version information" << endl
    << endl
    << "For more information, please visit the Oclgrind wiki page:" << endl
    << "-> https://github.com/jrprice/Oclgrind/wiki" << endl
    << endl;
}

static void setEnvironment(const char *name, const char *value)
{
#if defined(_WIN32) && !defined(__MINGW32__)
  _putenv_s(name, value);
#else
  setenv(name, value, 1);
#endif
}
