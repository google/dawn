RWByteAddressBuffer prevent_dce : register(u0, space2);

void atan_ad96e4() {
  float2 res = (0.78539818525314331055f).xx;
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atan_ad96e4();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atan_ad96e4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan_ad96e4();
  return;
}
