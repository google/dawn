struct S {
  float4 a;
  int b;
};


ByteAddressBuffer sb : register(t0);
S v(uint offset) {
  float4 v_1 = asfloat(sb.Load4((offset + 0u)));
  S v_2 = {v_1, asint(sb.Load((offset + 16u)))};
  return v_2;
}

[numthreads(1, 1, 1)]
void main() {
  S x = v(32u);
}

