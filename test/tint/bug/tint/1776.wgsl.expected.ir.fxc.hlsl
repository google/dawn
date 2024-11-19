struct S {
  float4 a;
  int b;
};


ByteAddressBuffer sb : register(t0);
S v(uint offset) {
  S v_1 = {asfloat(sb.Load4((offset + 0u))), asint(sb.Load((offset + 16u)))};
  return v_1;
}

[numthreads(1, 1, 1)]
void main() {
  S x = v(32u);
}

