cbuffer cbuffer_S : register(b0, space0) {
  uint4 S[4];
};

typedef int4 tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[4], uint offset) {
  int4 arr_1[4] = (int4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      const uint scalar_offset = ((offset + (i * 16u))) / 4;
      arr_1[i] = asint(buffer[scalar_offset / 4]);
    }
  }
  return arr_1;
}

typedef int4 func_S_arr_ret[4];
func_S_arr_ret func_S_arr() {
  return tint_symbol(S, 0u);
}

[numthreads(1, 1, 1)]
void main() {
  const int4 r[4] = func_S_arr();
  return;
}
