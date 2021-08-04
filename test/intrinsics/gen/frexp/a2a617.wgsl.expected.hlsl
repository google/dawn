intrinsics/gen/frexp/a2a617.wgsl:29:18 warning: use of deprecated intrinsic
  var res: f32 = frexp(1.0, &arg_1);
                 ^^^^^

float tint_frexp(float param_0, inout int param_1) {
  float float_exp;
  float significand = frexp(param_0, float_exp);
  param_1 = int(float_exp);
  return significand;
}

static int arg_1 = 0;

void frexp_a2a617() {
  float res = tint_frexp(1.0f, arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_a2a617();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_a2a617();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_a2a617();
  return;
}
