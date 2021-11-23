intrinsics/gen/isNan/1280ab.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec3<bool> = isNan(vec3<f32>());
                        ^^^^^

void isNan_1280ab() {
  bool3 res = isnan(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  isNan_1280ab();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  isNan_1280ab();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_1280ab();
  return;
}
