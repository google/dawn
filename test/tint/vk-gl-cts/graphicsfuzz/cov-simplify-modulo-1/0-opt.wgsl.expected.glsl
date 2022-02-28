vk-gl-cts/graphicsfuzz/cov-simplify-modulo-1/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<f32, 2>;
            ^^^^^^

vk-gl-cts/graphicsfuzz/cov-simplify-modulo-1/0-opt.wgsl:7:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr_1 = @stride(16) array<i32, 1>;
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
  tint_padded_array_element x_GLF_uniform_float_values[2];
};

struct tint_padded_array_element_1 {
  int el;
};

struct buf1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[1];
};

layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
} x_6;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 1) uniform buf1_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[1];
} x_8;

void main_1() {
  float a = 0.0f;
  float x_30 = x_6.x_GLF_uniform_float_values[0].el;
  a = tint_float_modulo(x_30, 1.0f);
  float x_32 = a;
  float x_34 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_32 == x_34)) {
    int x_40 = x_8.x_GLF_uniform_int_values[0].el;
    float x_42 = a;
    float x_43 = a;
    int x_45 = x_8.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_40), x_42, x_43, float(x_45));
  } else {
    float x_48 = a;
    x_GLF_color = vec4(x_48, x_48, x_48, x_48);
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
