intrinsics/gen/isNan/e4978e.wgsl:28:19 warning: use of deprecated intrinsic
  var res: bool = isNan(1.0);
                  ^^^^^

void isNan_e4978e() {
  bool res = isnan(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  isNan_e4978e();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  isNan_e4978e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_e4978e();
  return;
}
