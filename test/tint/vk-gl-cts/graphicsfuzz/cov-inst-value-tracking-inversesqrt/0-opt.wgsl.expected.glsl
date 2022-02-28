vk-gl-cts/graphicsfuzz/cov-inst-value-tracking-inversesqrt/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<f32, 2>;
            ^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  float el;
};

struct buf0 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
};

layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
} x_5;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  float x_23 = x_5.x_GLF_uniform_float_values[1].el;
  if ((inversesqrt(x_23) < -1.0f)) {
    float x_30 = x_5.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_30, x_30, x_30, x_30);
  } else {
    float x_33 = x_5.x_GLF_uniform_float_values[1].el;
    float x_35 = x_5.x_GLF_uniform_float_values[0].el;
    float x_37 = x_5.x_GLF_uniform_float_values[0].el;
    float x_39 = x_5.x_GLF_uniform_float_values[1].el;
    x_GLF_color = vec4(x_33, x_35, x_37, x_39);
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
