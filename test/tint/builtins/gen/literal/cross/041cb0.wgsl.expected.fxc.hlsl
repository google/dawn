void cross_041cb0() {
  float3 res = (0.0f).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  cross_041cb0();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  cross_041cb0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cross_041cb0();
  return;
}
