groupshared int arg_0;

void atomicCompareExchangeWeak_89ea3b() {
  int2 atomic_result = int2(0, 0);
  int atomic_compare_value = 1;
  InterlockedCompareExchange(arg_0, atomic_compare_value, 1, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  int2 res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_89ea3b();
  return;
}
