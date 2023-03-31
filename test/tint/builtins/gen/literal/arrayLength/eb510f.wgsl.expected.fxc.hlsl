RWByteAddressBuffer sb_rw : register(u0);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void arrayLength_eb510f() {
  uint tint_symbol_2 = 0u;
  sb_rw.GetDimensions(tint_symbol_2);
  const uint tint_symbol_3 = ((tint_symbol_2 - 0u) / 4u);
  uint res = tint_symbol_3;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  arrayLength_eb510f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  arrayLength_eb510f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  arrayLength_eb510f();
  return;
}
