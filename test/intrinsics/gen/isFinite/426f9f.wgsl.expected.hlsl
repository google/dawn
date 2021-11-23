intrinsics/gen/isFinite/426f9f.wgsl:28:19 warning: use of deprecated intrinsic
  var res: bool = isFinite(1.0);
                  ^^^^^^^^

void isFinite_426f9f() {
  bool res = isfinite(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  isFinite_426f9f();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  isFinite_426f9f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isFinite_426f9f();
  return;
}
