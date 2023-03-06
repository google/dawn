RWByteAddressBuffer prevent_dce : register(u0, space2);

void select_c4a4ef() {
  uint4 arg_0 = (1u).xxxx;
  uint4 arg_1 = (1u).xxxx;
  bool4 arg_2 = (true).xxxx;
  uint4 res = (arg_2 ? arg_1 : arg_0);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_c4a4ef();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_c4a4ef();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_c4a4ef();
  return;
}
