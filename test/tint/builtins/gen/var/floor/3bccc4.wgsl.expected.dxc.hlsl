RWByteAddressBuffer prevent_dce : register(u0, space2);

void floor_3bccc4() {
  float4 arg_0 = (1.5f).xxxx;
  float4 res = floor(arg_0);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  floor_3bccc4();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  floor_3bccc4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  floor_3bccc4();
  return;
}
