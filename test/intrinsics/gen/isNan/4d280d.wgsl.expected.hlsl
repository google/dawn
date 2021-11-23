intrinsics/gen/isNan/4d280d.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec4<bool> = isNan(vec4<f32>());
                        ^^^^^

void isNan_4d280d() {
  bool4 res = isnan(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  isNan_4d280d();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  isNan_4d280d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_4d280d();
  return;
}
