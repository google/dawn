uint tint_pack4x8unorm(float4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void pack4x8unorm_95c456() {
  float4 arg_0 = (1.0f).xxxx;
  uint res = tint_pack4x8unorm(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  pack4x8unorm_95c456();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  pack4x8unorm_95c456();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pack4x8unorm_95c456();
  return;
}
