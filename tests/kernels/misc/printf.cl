kernel void printf_kernel(
  const int i,
  const float f,
  const float4 f4v)
{
  printf("int = %d\n", i);
  printf("float = %.1f\n", f);
  printf("float4 = %.2v4f\n", f4v);
}
