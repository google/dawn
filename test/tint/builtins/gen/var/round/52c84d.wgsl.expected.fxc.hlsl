RWByteAddressBuffer prevent_dce : register(u0, space2);

void round_52c84d() {
  float2 arg_0 = (3.5f).xx;
  float2 res = round(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  round_52c84d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  round_52c84d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  round_52c84d();
  return;
}
