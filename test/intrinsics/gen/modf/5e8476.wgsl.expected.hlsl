intrinsics/gen/modf/5e8476.wgsl:29:18 warning: use of deprecated intrinsic
  var res: f32 = modf(1.0, &arg_1);
                 ^^^^

static float arg_1 = 0.0f;

void modf_5e8476() {
  float res = modf(1.0f, arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_5e8476();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_5e8476();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_5e8476();
  return;
}
