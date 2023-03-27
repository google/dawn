float tint_trunc(float param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void trunc_eb83df() {
  float arg_0 = 1.5f;
  float res = tint_trunc(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  trunc_eb83df();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  trunc_eb83df();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_eb83df();
  return;
}
