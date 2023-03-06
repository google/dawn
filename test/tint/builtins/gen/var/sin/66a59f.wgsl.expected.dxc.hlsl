RWByteAddressBuffer prevent_dce : register(u0, space2);

void sin_66a59f() {
  float16_t arg_0 = float16_t(1.5703125h);
  float16_t res = sin(arg_0);
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  sin_66a59f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  sin_66a59f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sin_66a59f();
  return;
}
