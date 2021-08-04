intrinsics/gen/frexp/2052e9.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = frexp(vec4<f32>(), &arg_1);
                       ^^^^^

float4 tint_frexp(float4 param_0, inout int4 param_1) {
  float4 float_exp;
  float4 significand = frexp(param_0, float_exp);
  param_1 = int4(float_exp);
  return significand;
}

void frexp_2052e9() {
  int4 arg_1 = int4(0, 0, 0, 0);
  float4 res = tint_frexp(float4(0.0f, 0.0f, 0.0f, 0.0f), arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_2052e9();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_2052e9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_2052e9();
  return;
}
