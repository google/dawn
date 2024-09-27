<dawn>/test/tint/diagnostic_filtering/case_body_attribute.wgsl:8:11 warning: 'textureSample' must only be called from uniform control flow
      _ = textureSample(t, s, vec2(0, 0));
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/case_body_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) {
  ^^^^^^

<dawn>/test/tint/diagnostic_filtering/case_body_attribute.wgsl:6:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) {
              ^

#version 310 es
precision highp float;
precision highp int;

int tint_ftoi(float v) {
  return mix(2147483647, mix(int(v), (-2147483647 - 1), (v < -2147483648.0f)), (v <= 2147483520.0f));
}

layout(location = 0) in float x_1;
uniform highp sampler2D t_s;

void tint_symbol(float x) {
  switch(tint_ftoi(x)) {
    case 0: {
      vec4 tint_phony = texture(t_s, vec2(0.0f));
      break;
    }
    default: {
      break;
    }
  }
}

void main() {
  tint_symbol(x_1);
  return;
}
