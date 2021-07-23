intrinsics/gen/modf/955651.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = modf(vec3<f32>(), &arg_1);
                       ^^^^

void modf_955651() {
  float3 arg_1 = float3(0.0f, 0.0f, 0.0f);
  float3 res = modf(float3(0.0f, 0.0f, 0.0f), arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  modf_955651();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  modf_955651();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_955651();
  return;
}
