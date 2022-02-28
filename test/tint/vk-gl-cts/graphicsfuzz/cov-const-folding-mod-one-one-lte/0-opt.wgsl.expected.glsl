vk-gl-cts/graphicsfuzz/cov-const-folding-mod-one-one-lte/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<i32, 2>;
            ^^^^^^

#version 310 es
precision mediump float;

float tint_float_modulo(float lhs, float rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  int el;
};

struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
};

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
} x_5;

void main_1() {
  if ((tint_float_modulo(1.0f, 1.0f) <= 0.01f)) {
    int x_29 = x_5.x_GLF_uniform_int_values[0].el;
    int x_32 = x_5.x_GLF_uniform_int_values[0].el;
    int x_35 = x_5.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(1.0f, float(x_29), float(x_32), float(x_35));
  } else {
    int x_39 = x_5.x_GLF_uniform_int_values[0].el;
    float x_40 = float(x_39);
    x_GLF_color = vec4(x_40, x_40, x_40, x_40);
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
