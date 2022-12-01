RWByteAddressBuffer S : register(u0, space0);

void tint_symbol(RWByteAddressBuffer buffer, uint offset, int value[4]) {
  int array[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      buffer.Store((offset + (i * 4u)), asuint(array[i]));
    }
  }
}

void func_S_arr() {
  const int tint_symbol_2[4] = (int[4])0;
  tint_symbol(S, 0u, tint_symbol_2);
}

[numthreads(1, 1, 1)]
void main() {
  func_S_arr();
  return;
}
