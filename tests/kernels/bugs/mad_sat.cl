kernel void entry(global long *res)
{
    res[0] = mad_sat(1L, -1L, 0L);
}
