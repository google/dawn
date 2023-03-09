diagnostic_filtering/function_attribute.wgsl:7:9 warning: 'textureSample' must only be called from uniform control flow
    _ = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

diagnostic_filtering/function_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

diagnostic_filtering/function_attribute.wgsl:6:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) {
      ^

#version 310 es
precision highp float;

layout(location = 0) in float x_1;
void tint_symbol(float x) {
  if ((x > 0.0f)) {
  }
}

void main() {
  tint_symbol(x_1);
  return;
}
