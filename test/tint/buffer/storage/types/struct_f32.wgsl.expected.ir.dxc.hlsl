struct Inner {
  float scalar_f32;
  float3 vec3_f32;
  float2x4 mat2x4_f32;
};

struct S {
  Inner inner;
};


ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, float2x4 obj) {
  tint_symbol_1.Store4((offset + 0u), asuint(obj[0u]));
  tint_symbol_1.Store4((offset + 16u), asuint(obj[1u]));
}

void v_1(uint offset, Inner obj) {
  tint_symbol_1.Store((offset + 0u), asuint(obj.scalar_f32));
  tint_symbol_1.Store3((offset + 16u), asuint(obj.vec3_f32));
  v((offset + 32u), obj.mat2x4_f32);
}

void v_2(uint offset, S obj) {
  Inner v_3 = obj.inner;
  v_1((offset + 0u), v_3);
}

float2x4 v_4(uint offset) {
  float4 v_5 = asfloat(tint_symbol.Load4((offset + 0u)));
  return float2x4(v_5, asfloat(tint_symbol.Load4((offset + 16u))));
}

Inner v_6(uint offset) {
  float v_7 = asfloat(tint_symbol.Load((offset + 0u)));
  float3 v_8 = asfloat(tint_symbol.Load3((offset + 16u)));
  Inner v_9 = {v_7, v_8, v_4((offset + 32u))};
  return v_9;
}

S v_10(uint offset) {
  Inner v_11 = v_6((offset + 0u));
  S v_12 = {v_11};
  return v_12;
}

[numthreads(1, 1, 1)]
void main() {
  S t = v_10(0u);
  v_2(0u, t);
}

