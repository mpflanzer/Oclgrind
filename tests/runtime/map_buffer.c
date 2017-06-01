#include "common.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOL 1e-8
#define MAX_ERRORS 8

const char *KERNEL_SOURCE =
"kernel void vecadd(global float *a, \n"
"                   global float *b, \n"
"                   global float *c) \n"
"{                                   \n"
"  int i = get_global_id(0);         \n"
"  c[i] = a[i] + b[i];               \n"
"}                                   \n"
;

unsigned checkResults(size_t N, float *a, float *b, float *results);

// Run everything as normal
unsigned run1(Context cl, cl_kernel kernel,
              cl_mem d_a, cl_mem d_b, cl_mem d_c, size_t N)
{
  cl_int err;
  float *h_a, *h_b, *h_c;
  size_t dataSize = N*sizeof(cl_float);

  // Initialise data
  srand(0);
  h_a = clEnqueueMapBuffer(cl.queue, d_a, CL_TRUE,
                           CL_MAP_WRITE_INVALIDATE_REGION,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_a buffer");
  h_b = clEnqueueMapBuffer(cl.queue, d_b, CL_TRUE,
                           CL_MAP_WRITE_INVALIDATE_REGION,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_b buffer");
  h_c = clEnqueueMapBuffer(cl.queue, d_c, CL_FALSE, CL_MAP_READ, 0, dataSize,
                           0, NULL, NULL, &err);
  checkError(err, "mapping d_c buffer");
  for (unsigned i = 0; i < N; i++)
  {
    h_a[i] = rand()/(float)RAND_MAX;
    h_b[i] = rand()/(float)RAND_MAX;
    h_c[i] = 0;
  }

  err = clEnqueueUnmapMemObject(cl.queue, d_a, h_a, 0, NULL, NULL);
  checkError(err, "unmapping d_a");
  err = clEnqueueUnmapMemObject(cl.queue, d_b, h_b, 0, NULL, NULL);
  checkError(err, "unmapping d_b");
  err = clEnqueueUnmapMemObject(cl.queue, d_c, h_c, 0, NULL, NULL);
  checkError(err, "unmapping d_c");

  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c);
  checkError(err, "setting kernel args");

  err = clEnqueueNDRangeKernel(cl.queue, kernel,
                               1, NULL, &N, NULL, 0, NULL, NULL);
  checkError(err, "enqueuing kernel");

  h_c = clEnqueueMapBuffer(cl.queue, d_c, CL_FALSE, CL_MAP_READ, 0, dataSize,
                           0, NULL, NULL, &err);
  checkError(err, "mapping d_c buffer");

  err = clFinish(cl.queue);
  checkError(err, "running kernel");

  unsigned errors = checkResults(N, h_a, h_b, h_c);

  err = clEnqueueUnmapMemObject(cl.queue, d_c, h_c, 0, NULL, NULL);
  checkError(err, "unmapping d_c");

  return errors;
}

// Don't unmap input buffers before running kernel
// Should result in "Invalid read from buffer mapped for writing" error
unsigned run2(Context cl, cl_kernel kernel,
              cl_mem d_a, cl_mem d_b, cl_mem d_c, size_t N)
{
  cl_int err;
  float *h_a, *h_b, *h_c;
  size_t dataSize = N*sizeof(cl_float);

  // Initialise data
  srand(0);
  h_a = clEnqueueMapBuffer(cl.queue, d_a, CL_TRUE,
                           CL_MAP_WRITE_INVALIDATE_REGION,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_a buffer");
  h_b = clEnqueueMapBuffer(cl.queue, d_b, CL_TRUE,
                           CL_MAP_WRITE_INVALIDATE_REGION,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_b buffer");
  h_c = clEnqueueMapBuffer(cl.queue, d_c, CL_FALSE, CL_MAP_READ, 0, dataSize,
                           0, NULL, NULL, &err);
  checkError(err, "mapping d_c buffer");
  for (unsigned i = 0; i < N; i++)
  {
    h_a[i] = rand()/(float)RAND_MAX;
    h_b[i] = rand()/(float)RAND_MAX;
    h_c[i] = 0;
  }

  err = clEnqueueUnmapMemObject(cl.queue, d_c, h_c, 0, NULL, NULL);
  checkError(err, "unmapping d_c");

  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c);
  checkError(err, "setting kernel args");

  err = clEnqueueNDRangeKernel(cl.queue, kernel,
                               1, NULL, &N, NULL, 0, NULL, NULL);
  checkError(err, "enqueuing kernel");

  h_c = clEnqueueMapBuffer(cl.queue, d_c, CL_FALSE, CL_MAP_READ, 0, dataSize,
                           0, NULL, NULL, &err);
  checkError(err, "mapping d_c buffer");

  err = clFinish(cl.queue);
  checkError(err, "running kernel");

  unsigned errors = checkResults(N, h_a, h_b, h_c);

  err = clEnqueueUnmapMemObject(cl.queue, d_a, h_a, 0, NULL, NULL);
  checkError(err, "unmapping d_a");
  err = clEnqueueUnmapMemObject(cl.queue, d_b, h_b, 0, NULL, NULL);
  checkError(err, "unmapping d_b");
  err = clEnqueueUnmapMemObject(cl.queue, d_c, h_c, 0, NULL, NULL);
  checkError(err, "unmapping d_c");

  return errors;
}

// Don't unmap output buffer before running kernel
// Should result in "Invalid write to mapped buffer" error
unsigned run3(Context cl, cl_kernel kernel,
              cl_mem d_a, cl_mem d_b, cl_mem d_c, size_t N)
{
  cl_int err;
  float *h_a, *h_b, *h_c;
  size_t dataSize = N*sizeof(cl_float);

  // Initialise data
  srand(0);
  h_a = clEnqueueMapBuffer(cl.queue, d_a, CL_TRUE,
                           CL_MAP_WRITE_INVALIDATE_REGION,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_a buffer");
  h_b = clEnqueueMapBuffer(cl.queue, d_b, CL_TRUE,
                           CL_MAP_WRITE_INVALIDATE_REGION,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_b buffer");
  h_c = clEnqueueMapBuffer(cl.queue, d_c, CL_FALSE, CL_MAP_READ, 0, dataSize,
                           0, NULL, NULL, &err);
  checkError(err, "mapping d_c buffer");
  for (unsigned i = 0; i < N; i++)
  {
    h_a[i] = rand()/(float)RAND_MAX;
    h_b[i] = rand()/(float)RAND_MAX;
    h_c[i] = 0;
  }

  err = clEnqueueUnmapMemObject(cl.queue, d_a, h_a, 0, NULL, NULL);
  checkError(err, "unmapping d_a");
  err = clEnqueueUnmapMemObject(cl.queue, d_b, h_b, 0, NULL, NULL);
  checkError(err, "unmapping d_b");

  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c);
  checkError(err, "setting kernel args");

  err = clEnqueueNDRangeKernel(cl.queue, kernel,
                               1, NULL, &N, NULL, 0, NULL, NULL);
  checkError(err, "enqueuing kernel");

  err = clFinish(cl.queue);
  checkError(err, "running kernel");

  unsigned errors = checkResults(N, h_a, h_b, h_c);

  err = clEnqueueUnmapMemObject(cl.queue, d_c, h_c, 0, NULL, NULL);
  checkError(err, "unmapping d_c");

  return errors;
}

// Re-map input buffers for reading
// Should not result in any error
unsigned run4(Context cl, cl_kernel kernel,
              cl_mem d_a, cl_mem d_b, cl_mem d_c, size_t N)
{
  cl_int err;
  float *h_a, *h_b, *h_c;
  size_t dataSize = N*sizeof(cl_float);

  // Initialise data
  srand(0);
  h_a = clEnqueueMapBuffer(cl.queue, d_a, CL_TRUE,
                           CL_MAP_WRITE_INVALIDATE_REGION,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_a buffer");
  h_b = clEnqueueMapBuffer(cl.queue, d_b, CL_TRUE,
                           CL_MAP_WRITE_INVALIDATE_REGION,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_b buffer");
  h_c = clEnqueueMapBuffer(cl.queue, d_c, CL_FALSE, CL_MAP_READ, 0, dataSize,
                           0, NULL, NULL, &err);
  checkError(err, "mapping d_c buffer");
  for (unsigned i = 0; i < N; i++)
  {
    h_a[i] = rand()/(float)RAND_MAX;
    h_b[i] = rand()/(float)RAND_MAX;
    h_c[i] = 0;
  }

  h_a = clEnqueueMapBuffer(cl.queue, d_a, CL_TRUE,
                           CL_MAP_READ,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_a buffer");
  h_b = clEnqueueMapBuffer(cl.queue, d_b, CL_TRUE,
                           CL_MAP_READ,
                           0, dataSize, 0, NULL, NULL, &err);
  checkError(err, "mapping d_b buffer");

  err = clEnqueueUnmapMemObject(cl.queue, d_a, h_a, 0, NULL, NULL);
  checkError(err, "unmapping d_a");
  err = clEnqueueUnmapMemObject(cl.queue, d_b, h_b, 0, NULL, NULL);
  checkError(err, "unmapping d_b");
  err = clEnqueueUnmapMemObject(cl.queue, d_c, h_c, 0, NULL, NULL);
  checkError(err, "unmapping d_c");

  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c);
  checkError(err, "setting kernel args");

  err = clEnqueueNDRangeKernel(cl.queue, kernel,
                               1, NULL, &N, NULL, 0, NULL, NULL);
  checkError(err, "enqueuing kernel");

  h_c = clEnqueueMapBuffer(cl.queue, d_c, CL_FALSE, CL_MAP_READ, 0, dataSize,
                           0, NULL, NULL, &err);
  checkError(err, "mapping d_c buffer");

  err = clFinish(cl.queue);
  checkError(err, "running kernel");

  unsigned errors = checkResults(N, h_a, h_b, h_c);

  err = clEnqueueUnmapMemObject(cl.queue, d_c, h_c, 0, NULL, NULL);
  checkError(err, "unmapping d_c");

  return errors;
}

int main(int argc, char *argv[])
{
  cl_int err;
  cl_kernel kernel;
  cl_mem d_a, d_b, d_c;

  size_t N = 1;
  if (argc > 1)
  {
    N = atoi(argv[1]);
  }

  Context cl = createContext(KERNEL_SOURCE, "");

  kernel = clCreateKernel(cl.program, "vecadd", &err);
  checkError(err, "creating kernel");

  size_t dataSize = N*sizeof(cl_float);

  d_a = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, dataSize, NULL, &err);
  checkError(err, "creating d_a buffer");
  d_b = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, dataSize, NULL, &err);
  checkError(err, "creating d_b buffer");
  d_c = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, dataSize, NULL, &err);
  checkError(err, "creating d_c buffer");

  unsigned errors = 0;

  errors += run1(cl, kernel, d_a, d_b, d_c, N);
  errors += run2(cl, kernel, d_a, d_b, d_c, N);
  errors += run3(cl, kernel, d_a, d_b, d_c, N);
  errors += run4(cl, kernel, d_a, d_b, d_c, N);

  clReleaseMemObject(d_a);
  clReleaseMemObject(d_b);
  clReleaseMemObject(d_c);
  clReleaseKernel(kernel);
  releaseContext(cl);

  return (errors != 0);
}

unsigned checkResults(size_t N, float *a, float *b, float *results)
{
  // Check results
  unsigned errors = 0;
  for (unsigned i = 0; i < N; i++)
  {
    float ref = a[i] + b[i];
    if (fabs(ref - results[i]) > TOL)
    {
      if (errors < MAX_ERRORS)
      {
        fprintf(stderr, "%4d: %.4f != %.4f\n", i, results[i], ref);
      }
      errors++;
    }
  }
  if (errors)
    printf("%d errors detected\n", errors);

  return errors;
}
