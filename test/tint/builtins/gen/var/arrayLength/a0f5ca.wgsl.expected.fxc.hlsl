ByteAddressBuffer sb_ro : register(t1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void arrayLength_a0f5ca() {
  uint tint_symbol_2 = 0u;
  sb_ro.GetDimensions(tint_symbol_2);
  const uint tint_symbol_3 = ((tint_symbol_2 - 0u) / 4u);
  uint res = tint_symbol_3;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  arrayLength_a0f5ca();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  arrayLength_a0f5ca();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  arrayLength_a0f5ca();
  return;
}
