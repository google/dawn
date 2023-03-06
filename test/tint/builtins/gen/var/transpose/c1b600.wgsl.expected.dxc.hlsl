RWByteAddressBuffer prevent_dce : register(u0, space2);

void prevent_dce_store(uint offset, float4x4 value) {
  prevent_dce.Store4((offset + 0u), asuint(value[0u]));
  prevent_dce.Store4((offset + 16u), asuint(value[1u]));
  prevent_dce.Store4((offset + 32u), asuint(value[2u]));
  prevent_dce.Store4((offset + 48u), asuint(value[3u]));
}

void transpose_c1b600() {
  float4x4 arg_0 = float4x4((1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx);
  float4x4 res = transpose(arg_0);
  prevent_dce_store(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_c1b600();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_c1b600();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_c1b600();
  return;
}
