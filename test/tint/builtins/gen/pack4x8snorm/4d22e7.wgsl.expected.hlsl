uint tint_pack4x8snorm(float4 param_0) {
  int4 i = int4(round(clamp(param_0, -1.0, 1.0) * 127.0)) & 0xff;
  return asuint(i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

void pack4x8snorm_4d22e7() {
  uint res = tint_pack4x8snorm(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  pack4x8snorm_4d22e7();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  pack4x8snorm_4d22e7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pack4x8snorm_4d22e7();
  return;
}
