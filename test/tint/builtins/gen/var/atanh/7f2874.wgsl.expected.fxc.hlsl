void atanh_7f2874() {
  float3 res = (0.54930615425109863281f).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_7f2874();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_7f2874();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_7f2874();
  return;
}
