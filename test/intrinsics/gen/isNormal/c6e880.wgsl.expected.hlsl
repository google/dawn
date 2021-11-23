intrinsics/gen/isNormal/c6e880.wgsl:28:19 warning: use of deprecated intrinsic
  var res: bool = isNormal(1.0);
                  ^^^^^^^^

bool tint_isNormal(float param_0) {
  uint exponent = asuint(param_0) & 0x7f80000;
  uint clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}

void isNormal_c6e880() {
  bool res = tint_isNormal(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  isNormal_c6e880();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  isNormal_c6e880();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNormal_c6e880();
  return;
}
