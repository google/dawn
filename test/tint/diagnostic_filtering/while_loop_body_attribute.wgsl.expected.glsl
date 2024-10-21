<dawn>/test/tint/diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 warning: 'textureSample' must only be called from uniform control flow
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/while_loop_body_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  while (x > v.x) @diagnostic(warning, derivative_uniformity) {
  ^^^^^

<dawn>/test/tint/diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 note: return value of 'textureSample' may be non-uniform
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D t_s;
layout(location = 0) in float tint_symbol_loc0_Input;
void tint_symbol_inner(float x) {
  vec4 v = vec4(0.0f);
  {
    while(true) {
      if ((x > v.x)) {
      } else {
        break;
      }
      v = texture(t_s, vec2(0.0f));
      {
      }
      continue;
    }
  }
}
void main() {
  tint_symbol_inner(tint_symbol_loc0_Input);
}
