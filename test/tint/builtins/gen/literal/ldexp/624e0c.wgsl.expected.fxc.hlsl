SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void ldexp_624e0c() {
  float16_t res = float16_t(2.0h);
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ldexp_624e0c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ldexp_624e0c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_624e0c();
  return;
}
