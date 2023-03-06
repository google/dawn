float3 tint_sinh(float3 x) {
  return log((x + sqrt(((x * x) + 1.0f))));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void asinh_2265ee() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = tint_sinh(arg_0);
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  asinh_2265ee();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  asinh_2265ee();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asinh_2265ee();
  return;
}
