SKIP: FAILED

float16_t tint_sinh(float16_t x) {
  return log((x + sqrt(((x * x) + float16_t(1.0h)))));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void asinh_468a48() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t res = tint_sinh(arg_0);
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  asinh_468a48();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  asinh_468a48();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asinh_468a48();
  return;
}
