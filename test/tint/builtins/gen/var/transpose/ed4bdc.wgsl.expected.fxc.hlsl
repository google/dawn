void transpose_ed4bdc() {
  float3x2 arg_0 = float3x2((1.0f).xx, (1.0f).xx, (1.0f).xx);
  float2x3 res = transpose(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_ed4bdc();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_ed4bdc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_ed4bdc();
  return;
}
