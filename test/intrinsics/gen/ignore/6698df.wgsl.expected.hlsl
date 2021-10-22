intrinsics/gen/ignore/6698df.wgsl:28:3 warning: use of deprecated intrinsic
  ignore(1u);
  ^^^^^^

void ignore_6698df() {
  1u;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ignore_6698df();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ignore_6698df();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ignore_6698df();
  return;
}
