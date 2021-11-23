intrinsics/gen/isInf/a46d6f.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec2<bool> = isInf(vec2<f32>());
                        ^^^^^

void isInf_a46d6f() {
  bool2 res = isinf(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  isInf_a46d6f();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  isInf_a46d6f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isInf_a46d6f();
  return;
}
