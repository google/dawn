SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  float el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
} x_5;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float x_23 = x_5.x_GLF_uniform_float_values[1].el;
  if ((rsqrt(x_23) < -1.0f)) {
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
struct tint_symbol_1 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_inner() {
  main_1();
  main_out tint_symbol_2 = main_out(x_GLF_color);
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  main_out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:18: 'rsqrt' : no matching overloaded function found 
ERROR: 0:18: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



