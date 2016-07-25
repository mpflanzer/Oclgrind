kernel void rhadd_overflow(global ulong *res)
{
    res[0] = rhadd(0UL, 0xFFFFFFFFFFFFFFFFUL);
    res[1] = hadd(1UL, 0xFFFFFFFFFFFFFFFFUL);
}
