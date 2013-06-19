#ifndef _ICD_H_
#define _ICD_H_

// Need to rename all CL API functions to prevent ICD loader functions calling
// themselves via the dispatch table. Include this before cl headers.
#include "rename_api.h"

#include <CL/cl.h>

#ifndef CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#endif

#ifndef CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#endif

#define CL_PLATFORM_ICD_SUFFIX_KHR 0x0920

namespace spirsim
{
  class Program;
}

typedef struct CLIicdDispatchTable_st
{
  void *entries[256];
  int entryCount;
} CLIicdDispatchTable;

struct _cl_platform_id
{
  CLIicdDispatchTable *dispatch;
  const char *profile;
  const char *version;
  const char *name;
  const char *vendor;
  const char *extensions;
  const char *suffix;
};

struct _cl_device_id
{
  CLIicdDispatchTable* dispatch;
};

struct _cl_context
{
  CLIicdDispatchTable* dispatch;
};

struct _cl_command_queue
{
  CLIicdDispatchTable* dispatch;
};

struct _cl_mem
{
  CLIicdDispatchTable* dispatch;
};

struct _cl_program
{
  CLIicdDispatchTable* dispatch;
  spirsim::Program *program;
};

struct _cl_kernel
{
  CLIicdDispatchTable* dispatch;
};

struct _cl_event
{
  CLIicdDispatchTable* dispatch;
};

struct _cl_sampler
{
  CLIicdDispatchTable* dispatch;
};

cl_int cliIcdDispatchTableCreate(CLIicdDispatchTable **outDispatchTable);

CL_API_ENTRY cl_int CL_API_CALL
clIcdGetPlatformIDsKHR(cl_uint num_entries,
                       cl_platform_id *platforms,
                       cl_uint *num_platforms);

#endif /* _ICD_STRUCTS_H_ */