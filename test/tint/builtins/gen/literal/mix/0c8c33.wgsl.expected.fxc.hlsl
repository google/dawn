RWByteAddressBuffer prevent_dce : register(u0, space2);

void mix_0c8c33() {
  float3 res = (1.0f).xxx;
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  mix_0c8c33();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  mix_0c8c33();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_0c8c33();
  return;
}
