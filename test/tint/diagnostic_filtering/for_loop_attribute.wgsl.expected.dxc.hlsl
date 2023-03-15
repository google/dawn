diagnostic_filtering/for_loop_attribute.wgsl:5:21 warning: 'dpdx' must only be called from uniform control flow
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
                    ^^^^^^^^^

diagnostic_filtering/for_loop_attribute.wgsl:5:3 note: control flow depends on possibly non-uniform value
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
  ^^^

diagnostic_filtering/for_loop_attribute.wgsl:5:21 note: return value of 'dpdx' may be non-uniform
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
                    ^^^^^^^^^

struct tint_symbol_1 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  float4 v = (0.0f).xxxx;
  {
    while (true) {
      bool tint_tmp = (x > v.x);
      if (tint_tmp) {
        tint_tmp = (ddx(1.0f) > 0.0f);
      }
      if (!((tint_tmp))) { break; }
    }
  }
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x);
  return;
}
