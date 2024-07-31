struct S {
  float f;
};


ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, S obj) {
  tint_symbol_1.Store((offset + 0u), asuint(obj.f));
}

S v_1(uint offset) {
  S v_2 = {asfloat(tint_symbol.Load((offset + 0u)))};
  return v_2;
}

[numthreads(1, 1, 1)]
void main() {
  S v_3 = v_1(0u);
  v(0u, v_3);
}

