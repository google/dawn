struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


RWByteAddressBuffer a : register(u0);
[numthreads(1, 1, 1)]
void compute_main() {
  int v_1 = 0;
  a.InterlockedCompareExchange(int(0u), 1, 1, v_1);
  int v_2 = v_1;
  atomic_compare_exchange_result_i32 v_3 = {v_2, (v_2 == 1)};
  int v = v_3.old_value;
}

