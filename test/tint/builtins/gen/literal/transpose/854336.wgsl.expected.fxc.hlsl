RWByteAddressBuffer prevent_dce : register(u0, space2);

void prevent_dce_store(uint offset, float3x3 value) {
  prevent_dce.Store3((offset + 0u), asuint(value[0u]));
  prevent_dce.Store3((offset + 16u), asuint(value[1u]));
  prevent_dce.Store3((offset + 32u), asuint(value[2u]));
}

void transpose_854336() {
  float3x3 res = float3x3((1.0f).xxx, (1.0f).xxx, (1.0f).xxx);
  prevent_dce_store(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_854336();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_854336();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_854336();
  return;
}
