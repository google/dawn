intrinsics/gen/frexp/a951b5.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec2<f32> = frexp(vec2<f32>(), &arg_1);
                       ^^^^^

float2 tint_frexp(float2 param_0, inout int2 param_1) {
  float2 float_exp;
  float2 significand = frexp(param_0, float_exp);
  param_1 = int2(float_exp);
  return significand;
}

void frexp_a951b5() {
  int2 arg_1 = int2(0, 0);
  float2 res = tint_frexp(float2(0.0f, 0.0f), arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_a951b5();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_a951b5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_a951b5();
  return;
}
