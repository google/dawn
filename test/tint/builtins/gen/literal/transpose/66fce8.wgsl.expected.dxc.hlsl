void transpose_66fce8() {
  float3x3 res = float3x3((1.0f).xxx, (1.0f).xxx, (1.0f).xxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_66fce8();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_66fce8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_66fce8();
  return;
}
