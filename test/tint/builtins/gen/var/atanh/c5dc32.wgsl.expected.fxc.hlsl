void atanh_c5dc32() {
  float res = 0.54930615425109863281f;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_c5dc32();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_c5dc32();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_c5dc32();
  return;
}
