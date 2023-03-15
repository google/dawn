diagnostic_filtering/while_loop_attribute.wgsl:5:21 warning: 'dpdx' must only be called from uniform control flow
  while (x > 0.0 && dpdx(1.0) > 0.0)  {
                    ^^^^^^^^^

diagnostic_filtering/while_loop_attribute.wgsl:5:3 note: control flow depends on possibly non-uniform value
  while (x > 0.0 && dpdx(1.0) > 0.0)  {
  ^^^^^

diagnostic_filtering/while_loop_attribute.wgsl:5:21 note: return value of 'dpdx' may be non-uniform
  while (x > 0.0 && dpdx(1.0) > 0.0)  {
                    ^^^^^^^^^

#version 310 es
precision highp float;

layout(location = 0) in float x_1;
void tint_symbol(float x) {
  vec4 v = vec4(0.0f);
  while (true) {
    bool tint_tmp = (x > 0.0f);
    if (tint_tmp) {
      tint_tmp = (dFdx(1.0f) > 0.0f);
    }
    if (!((tint_tmp))) { break; }
  }
}

void main() {
  tint_symbol(x_1);
  return;
}
