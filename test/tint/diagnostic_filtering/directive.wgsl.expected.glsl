<dawn>/test/tint/diagnostic_filtering/directive.wgsl:9:9 warning: 'textureSample' must only be called from uniform control flow
    _ = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/directive.wgsl:8:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

<dawn>/test/tint/diagnostic_filtering/directive.wgsl:8:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) {
      ^

#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in float x_1;
uniform highp sampler2D t_s;

void tint_symbol(float x) {
  if ((x > 0.0f)) {
    vec4 tint_phony = texture(t_s, vec2(0.0f));
  }
}

void main() {
  tint_symbol(x_1);
  return;
}
