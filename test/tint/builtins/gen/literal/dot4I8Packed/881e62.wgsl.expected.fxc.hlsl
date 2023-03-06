SKIP: FAILED

int tint_dot4I8Packed(uint param_0, uint param_1) {
  int accumulator = 0;
  return dot4add_i8packed(param_0, param_1, accumulator);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void dot4I8Packed_881e62() {
  int res = tint_dot4I8Packed(1u, 1u);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  dot4I8Packed_881e62();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  dot4I8Packed_881e62();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot4I8Packed_881e62();
  return;
}
