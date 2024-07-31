struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


RWByteAddressBuffer a : register(u0);
[numthreads(16, 1, 1)]
void main() {
  uint value = 42u;
  uint v = value;
  uint v_1 = 0u;
  a.InterlockedCompareExchange(uint(0u), 0u, v, v_1);
  uint v_2 = v_1;
  atomic_compare_exchange_result_u32 result = {v_2, (v_2 == 0u)};
}

