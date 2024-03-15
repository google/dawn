<dawn>/test/tint/diagnostic_filtering/for_loop_attribute.wgsl:5:21 warning: 'dpdx' must only be called from uniform control flow
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
                    ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_attribute.wgsl:5:3 note: control flow depends on possibly non-uniform value
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
  ^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_attribute.wgsl:5:21 note: return value of 'dpdx' may be non-uniform
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
                    ^^^^^^^^^

#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in float x_1;
void tint_symbol(float x) {
  vec4 v = vec4(0.0f);
  while (true) {
    bool tint_symbol_1 = (x > v.x);
    if (tint_symbol_1) {
      float tint_symbol_2 = dFdx(1.0f);
      tint_symbol_1 = (tint_symbol_2 > 0.0f);
    }
    if (!(tint_symbol_1)) {
      break;
    }
    {
    }
  }
}

void main() {
  tint_symbol(x_1);
  return;
}
