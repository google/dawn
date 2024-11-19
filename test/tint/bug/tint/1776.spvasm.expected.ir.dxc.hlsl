struct S {
  float4 a;
  int b;
};


ByteAddressBuffer sb : register(t0);
S v(uint offset) {
  S v_1 = {asfloat(sb.Load4((offset + 0u))), asint(sb.Load((offset + 16u)))};
  return v_1;
}

void main_1() {
  S x_18 = v(32u);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

