SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  float el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_float_values[1];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[1];
} x_6;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  vec2 v1 = vec2(0.0f, 0.0f);
  vec2 b = vec2(0.0f, 0.0f);
  float a = 0.0f;
  bool x_51 = false;
  bool x_52_phi = false;
  float x_30 = x_6.x_GLF_uniform_float_values[0].el;
  v1 = vec2(x_30, x_30);
  b = frac(v1);
  a = smoothstep(vec2(1.0f, 1.0f), vec2(1.0f, 1.0f), b).x;
  float x_38 = x_6.x_GLF_uniform_float_values[0].el;
  float x_39 = a;
  float x_40 = a;
  float x_42 = x_6.x_GLF_uniform_float_values[0].el;
  x_GLF_color = vec4(x_38, x_39, x_40, x_42);
  float x_45 = b.x;
  bool x_46 = (x_45 < 1.0f);
  x_52_phi = x_46;
  if (x_46) {
    float x_50 = b.y;
    x_51 = (x_50 < 1.0f);
    x_52_phi = x_51;
  }
  if (x_52_phi) {
    float x_57 = x_6.x_GLF_uniform_float_values[0].el;
    float x_59 = b.x;
    float x_61 = b.y;
    float x_63 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_57, x_59, x_61, x_63);
  } else {
    float x_66 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_66, x_66, x_66, x_66);
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
ERROR: 0:24: 'frac' : no matching overloaded function found 
ERROR: 0:24: 'assign' :  cannot convert from ' const float' to ' temp mediump 2-component vector of float'
ERROR: 0:24: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



