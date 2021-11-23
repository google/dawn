intrinsics/gen/isNormal/b00ab1.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec2<bool> = isNormal(vec2<f32>());
                        ^^^^^^^^

bool2 tint_isNormal(float2 param_0) {
  uint2 exponent = asuint(param_0) & 0x7f80000;
  uint2 clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}

void isNormal_b00ab1() {
  bool2 res = tint_isNormal(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  isNormal_b00ab1();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  isNormal_b00ab1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNormal_b00ab1();
  return;
}
