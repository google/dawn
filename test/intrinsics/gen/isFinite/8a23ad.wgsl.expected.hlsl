intrinsics/gen/isFinite/8a23ad.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec3<bool> = isFinite(vec3<f32>());
                        ^^^^^^^^

void isFinite_8a23ad() {
  bool3 res = isfinite(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  isFinite_8a23ad();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  isFinite_8a23ad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isFinite_8a23ad();
  return;
}
