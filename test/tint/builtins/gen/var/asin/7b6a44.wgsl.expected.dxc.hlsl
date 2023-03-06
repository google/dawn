RWByteAddressBuffer prevent_dce : register(u0, space2);

void asin_7b6a44() {
  float2 arg_0 = (0.47942554950714111328f).xx;
  float2 res = asin(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  asin_7b6a44();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  asin_7b6a44();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_7b6a44();
  return;
}
