float16_t tint_atanh(float16_t x) {
  return (log(((float16_t(1.0h) + x) / (float16_t(1.0h) - x))) * float16_t(0.5h));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void atanh_d2d8cd() {
  float16_t arg_0 = float16_t(0.5h);
  float16_t res = tint_atanh(arg_0);
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_d2d8cd();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_d2d8cd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_d2d8cd();
  return;
}
