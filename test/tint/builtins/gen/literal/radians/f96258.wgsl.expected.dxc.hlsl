RWByteAddressBuffer prevent_dce : register(u0, space2);

void radians_f96258() {
  float3 res = (0.01745329238474369049f).xxx;
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  radians_f96258();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  radians_f96258();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  radians_f96258();
  return;
}
