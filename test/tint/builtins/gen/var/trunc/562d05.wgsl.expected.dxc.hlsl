void trunc_562d05() {
  float3 arg_0 = (1.5f).xxx;
  float3 res = trunc(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  trunc_562d05();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  trunc_562d05();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_562d05();
  return;
}
