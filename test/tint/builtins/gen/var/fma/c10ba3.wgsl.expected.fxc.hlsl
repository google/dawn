RWByteAddressBuffer prevent_dce : register(u0, space2);

void fma_c10ba3() {
  float arg_0 = 1.0f;
  float arg_1 = 1.0f;
  float arg_2 = 1.0f;
  float res = mad(arg_0, arg_1, arg_2);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  fma_c10ba3();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  fma_c10ba3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fma_c10ba3();
  return;
}
