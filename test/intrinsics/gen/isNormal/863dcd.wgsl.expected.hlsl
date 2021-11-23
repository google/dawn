intrinsics/gen/isNormal/863dcd.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec4<bool> = isNormal(vec4<f32>());
                        ^^^^^^^^

bool4 tint_isNormal(float4 param_0) {
  uint4 exponent = asuint(param_0) & 0x7f80000;
  uint4 clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}

void isNormal_863dcd() {
  bool4 res = tint_isNormal(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  isNormal_863dcd();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  isNormal_863dcd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNormal_863dcd();
  return;
}
