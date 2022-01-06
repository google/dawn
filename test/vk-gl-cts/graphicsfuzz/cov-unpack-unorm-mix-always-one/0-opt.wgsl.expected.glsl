SKIP: FAILED

#version 310 es
precision mediump float;

vec4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}


struct tint_padded_array_element {
  uint el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_uint_values[1];
};
struct tint_padded_array_element_1 {
  int el;
};
struct buf1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[2];
};
struct tint_padded_array_element_2 {
  float el;
};
struct buf2 {
  tint_padded_array_element_2 x_GLF_uniform_float_values[3];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_uint_values[1];
} x_6;
layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[2];
} x_8;
layout (binding = 2) uniform buf2_1 {
  tint_padded_array_element_2 x_GLF_uniform_float_values[3];
} x_10;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  vec4 v = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  uint x_39 = x_6.x_GLF_uniform_uint_values[0].el;
  uint x_41 = x_6.x_GLF_uniform_uint_values[0].el;
  v = tint_unpack4x8unorm((x_39 / (true ? 92382u : x_41)));
  vec4 x_45 = v;
  int x_47 = x_8.x_GLF_uniform_int_values[0].el;
  int x_50 = x_8.x_GLF_uniform_int_values[0].el;
  int x_53 = x_8.x_GLF_uniform_int_values[0].el;
  float x_56 = x_10.x_GLF_uniform_float_values[1].el;
  float x_58 = x_10.x_GLF_uniform_float_values[2].el;
  float x_63 = x_10.x_GLF_uniform_float_values[0].el;
  if ((distance(x_45, vec4(float(x_47), float(x_50), float(x_53), (x_56 / x_58))) < x_63)) {
    int x_69 = x_8.x_GLF_uniform_int_values[1].el;
    int x_72 = x_8.x_GLF_uniform_int_values[0].el;
    int x_75 = x_8.x_GLF_uniform_int_values[0].el;
    int x_78 = x_8.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(float(x_69), float(x_72), float(x_75), float(x_78));
  } else {
    int x_82 = x_8.x_GLF_uniform_int_values[0].el;
    float x_83 = float(x_82);
    x_GLF_color = vec4(x_83, x_83, x_83, x_83);
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
ERROR: 0:6: 'uint4' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



