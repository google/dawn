RWByteAddressBuffer prevent_dce : register(u0, space2);

void prevent_dce_store(uint offset, float3x4 value) {
  prevent_dce.Store4((offset + 0u), asuint(value[0u]));
  prevent_dce.Store4((offset + 16u), asuint(value[1u]));
  prevent_dce.Store4((offset + 32u), asuint(value[2u]));
}

void transpose_2585cd() {
  float3x4 res = float3x4((1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx);
  prevent_dce_store(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_2585cd();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_2585cd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_2585cd();
  return;
}
