<dawn>/test/tint/diagnostic_filtering/else_body_attribute.wgsl:8:9 warning: 'textureSample' must only be called from uniform control flow
    _ = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/else_body_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

<dawn>/test/tint/diagnostic_filtering/else_body_attribute.wgsl:6:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) {
      ^

#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
layout(location = 0) in float tint_interstage_location0;
void main_inner(float x) {
  if ((x > 0.0f)) {
  } else {
    texture(f_t_s, vec2(0.0f));
  }
}
void main() {
  main_inner(tint_interstage_location0);
}
