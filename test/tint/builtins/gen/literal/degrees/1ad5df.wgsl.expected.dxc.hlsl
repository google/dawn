RWByteAddressBuffer prevent_dce : register(u0, space2);

void degrees_1ad5df() {
  float2 res = (57.2957763671875f).xx;
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  degrees_1ad5df();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  degrees_1ad5df();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  degrees_1ad5df();
  return;
}
