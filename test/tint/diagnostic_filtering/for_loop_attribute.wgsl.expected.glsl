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

layout(location = 0) in float tint_interstage_location0;
void main_inner(float x) {
  vec4 v = vec4(0.0f);
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
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
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
      }
      continue;
    }
  }
}
void main() {
  main_inner(tint_interstage_location0);
}
