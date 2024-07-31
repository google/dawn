struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_1bd40a() {
  int v = 0;
  sb_rw.InterlockedCompareExchange(int(0u), 1, 1, v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 res = {v_1, (v_1 == 1)};
}

void fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_1bd40a();
}

