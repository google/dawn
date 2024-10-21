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

layout(location = 0) in float tint_symbol_loc0_Input;
void tint_symbol_inner(float x) {
  vec4 v = vec4(0.0f);
  {
    while(true) {
      bool v_1 = false;
      if ((x > v.x)) {
        v_1 = (dFdx(1.0f) > 0.0f);
      } else {
        v_1 = false;
      }
      if (v_1) {
      } else {
        break;
      }
      {
      }
      continue;
    }
  }
}
void main() {
  tint_symbol_inner(tint_symbol_loc0_Input);
}
