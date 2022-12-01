ByteAddressBuffer S : register(t0, space0);

typedef int tint_symbol_ret[4];
tint_symbol_ret tint_symbol(ByteAddressBuffer buffer, uint offset) {
  int arr_1[4] = (int[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr_1[i] = asint(buffer.Load((offset + (i * 4u))));
    }
  }
  return arr_1;
}

typedef int func_S_arr_ret[4];
func_S_arr_ret func_S_arr() {
  return tint_symbol(S, 0u);
}

[numthreads(1, 1, 1)]
void main() {
  const int r[4] = func_S_arr();
  return;
}
