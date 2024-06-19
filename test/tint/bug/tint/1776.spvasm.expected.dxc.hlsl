struct S {
  float4 a;
  int b;
};

ByteAddressBuffer sb : register(t0);

S sb_load(uint offset) {
  S tint_symbol = {asfloat(sb.Load4((offset + 0u))), asint(sb.Load((offset + 16u)))};
  return tint_symbol;
}

void main_1() {
  S x_18 = sb_load(32u);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
