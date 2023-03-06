RWByteAddressBuffer prevent_dce : register(u0, space2);

void pow_04a908() {
  float4 arg_0 = (1.0f).xxxx;
  float4 arg_1 = (1.0f).xxxx;
  float4 res = pow(arg_0, arg_1);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  pow_04a908();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  pow_04a908();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pow_04a908();
  return;
}
