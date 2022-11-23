struct frexp_result_vec3_f32 {
  float3 fract;
  int3 exp;
};
void frexp_bf45ae() {
  frexp_result_vec3_f32 res = {(0.5f).xxx, (1).xxx};
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_bf45ae();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_bf45ae();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_bf45ae();
  return;
}
