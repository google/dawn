<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:5:11 warning: 'dpdx' must only be called from uniform control flow
      _ = dpdx(1.0);
          ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:3:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) @diagnostic(warning, derivative_uniformity) {
  ^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:3:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) @diagnostic(warning, derivative_uniformity) {
              ^

#version 310 es
precision highp float;
precision highp int;

int tint_ftoi(float v) {
  return mix(2147483647, mix(int(v), (-2147483647 - 1), (v < -2147483648.0f)), (v <= 2147483520.0f));
}

layout(location = 0) in float x_1;
void tint_symbol(float x) {
  switch(tint_ftoi(x)) {
    default: {
      float tint_phony = dFdx(1.0f);
      break;
    }
  }
}

void main() {
  tint_symbol(x_1);
  return;
}
