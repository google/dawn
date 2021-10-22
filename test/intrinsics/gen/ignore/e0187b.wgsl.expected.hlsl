intrinsics/gen/ignore/e0187b.wgsl:29:3 warning: use of deprecated intrinsic
  ignore(arg_0);
  ^^^^^^

Texture2D arg_0 : register(t0, space1);

void ignore_e0187b() {
  arg_0;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ignore_e0187b();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ignore_e0187b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ignore_e0187b();
  return;
}
