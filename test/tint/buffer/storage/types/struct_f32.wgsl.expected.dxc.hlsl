struct Inner {
  float scalar_f32;
  float3 vec3_f32;
  float2x4 mat2x4_f32;
};
struct S {
  Inner inner;
};

ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

float2x4 tint_symbol_6(ByteAddressBuffer buffer, uint offset) {
  return float2x4(asfloat(buffer.Load4((offset + 0u))), asfloat(buffer.Load4((offset + 16u))));
}

Inner tint_symbol_3(ByteAddressBuffer buffer, uint offset) {
  const Inner tint_symbol_14 = {asfloat(buffer.Load((offset + 0u))), asfloat(buffer.Load3((offset + 16u))), tint_symbol_6(buffer, (offset + 32u))};
  return tint_symbol_14;
}

S tint_symbol_2(ByteAddressBuffer buffer, uint offset) {
  const S tint_symbol_15 = {tint_symbol_3(buffer, (offset + 0u))};
  return tint_symbol_15;
}

void tint_symbol_12(RWByteAddressBuffer buffer, uint offset, float2x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
}

void tint_symbol_9(RWByteAddressBuffer buffer, uint offset, Inner value) {
  buffer.Store((offset + 0u), asuint(value.scalar_f32));
  buffer.Store3((offset + 16u), asuint(value.vec3_f32));
  tint_symbol_12(buffer, (offset + 32u), value.mat2x4_f32);
}

void tint_symbol_8(RWByteAddressBuffer buffer, uint offset, S value) {
  tint_symbol_9(buffer, (offset + 0u), value.inner);
}

[numthreads(1, 1, 1)]
void main() {
  const S t = tint_symbol_2(tint_symbol, 0u);
  tint_symbol_8(tint_symbol_1, 0u, t);
  return;
}
