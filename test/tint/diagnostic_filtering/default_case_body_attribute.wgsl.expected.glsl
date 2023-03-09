diagnostic_filtering/default_case_body_attribute.wgsl:8:11 warning: 'textureSample' must only be called from uniform control flow
      _ = textureSample(t, s, vec2(0, 0));
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

diagnostic_filtering/default_case_body_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) {
  ^^^^^^

diagnostic_filtering/default_case_body_attribute.wgsl:6:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) {
              ^

#version 310 es
precision highp float;

layout(location = 0) in float x_1;
int tint_ftoi(float v) {
  return ((v < 2147483520.0f) ? ((v < -2147483648.0f) ? (-2147483647 - 1) : int(v)) : 2147483647);
}

void tint_symbol(float x) {
  switch(tint_ftoi(x)) {
    default: {
      break;
    }
  }
}

void main() {
  tint_symbol(x_1);
  return;
}
