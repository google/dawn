RWByteAddressBuffer sb_rw : register(u0, space0);

void arrayLength_61b1c7() {
  uint tint_symbol_2 = 0u;
  sb_rw.GetDimensions(tint_symbol_2);
  const uint tint_symbol_3 = ((tint_symbol_2 - 0u) / 4u);
  uint res = tint_symbol_3;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  arrayLength_61b1c7();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  arrayLength_61b1c7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  arrayLength_61b1c7();
  return;
}
