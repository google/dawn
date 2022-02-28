vk-gl-cts/graphicsfuzz/cov-mod-uint-bits-float/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<f32, 3>;
            ^^^^^^

#version 310 es
precision mediump float;

float tint_float_modulo(float lhs, float rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  float el;
};

struct buf0 {
  tint_padded_array_element x_GLF_uniform_float_values[3];
};

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[3];
} x_6;

void main_1() {
  float a = 0.0f;
  a = tint_float_modulo(uintBitsToFloat(1u), 1.0f);
  float x_29 = x_6.x_GLF_uniform_float_values[1].el;
  x_GLF_color = vec4(x_29, x_29, x_29, x_29);
  float x_31 = a;
  float x_33 = x_6.x_GLF_uniform_float_values[2].el;
  if ((x_31 < x_33)) {
    float x_38 = x_6.x_GLF_uniform_float_values[0].el;
    float x_40 = x_6.x_GLF_uniform_float_values[1].el;
    float x_42 = x_6.x_GLF_uniform_float_values[1].el;
    float x_44 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_38, x_40, x_42, x_44);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};

main_out tint_symbol() {
  main_1();
  main_out tint_symbol_1 = main_out(x_GLF_color);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol();
  x_GLF_color_1_1 = inner_result.x_GLF_color_1;
  return;
}
