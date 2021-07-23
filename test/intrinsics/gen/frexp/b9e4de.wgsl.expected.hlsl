intrinsics/gen/frexp/b9e4de.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = frexp(vec3<f32>(), &arg_1);
                       ^^^^^

float3 tint_frexp(float3 param_0, inout int3 param_1) {
  float3 float_exp;
  float3 significand = frexp(param_0, float_exp);
  param_1 = int3(float_exp);
  return significand;
}

void frexp_b9e4de() {
  int3 arg_1 = int3(0, 0, 0);
  float3 res = tint_frexp(float3(0.0f, 0.0f, 0.0f), arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_b9e4de();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_b9e4de();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_b9e4de();
  return;
}
