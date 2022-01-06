SKIP: FAILED

warning: use of deprecated intrinsic
#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf1 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
};
struct tint_padded_array_element_1 {
  float el;
};
struct buf0 {
  tint_padded_array_element_1 x_GLF_uniform_float_values[1];
};

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
} x_8;

void main_1() {
  float f0 = 0.0f;
  float s1 = 0.0f;
  float f1 = 0.0f;
  bool x_72 = false;
  bool x_73_phi = false;
  f0 = (10.0f - (0.000001f * floor((10.0f / 0.000001f))));
  s1 = 9.99999935e-39f;
  if ((s1 == 0.0f)) {
    s1 = 1.0f;
  }
  bool x_62 = false;
  bool x_71 = false;
  bool x_63_phi = false;
  bool x_72_phi = false;
  float x_42 = s1;
  f1 = (10.0f - (x_42 * floor((10.0f / x_42))));
  bool x_48 = (isinf(f1) | (s1 == 1.0f));
  x_73_phi = x_48;
  if (!(x_48)) {
    bool x_54 = (f0 == f1);
    x_63_phi = x_54;
    if (!(x_54)) {
      x_62 = ((f0 > 0.99000001f) & (f0 < 0.01f));
      x_63_phi = x_62;
    }
    bool x_63 = x_63_phi;
    x_72_phi = x_63;
    if (!(x_63)) {
      x_71 = ((f1 > 0.99000001f) & (f1 < 0.01f));
      x_72_phi = x_71;
    }
    x_72 = x_72_phi;
    x_73_phi = x_72;
  }
  if ((x_73_phi | (f1 == 10.0f))) {
    int x_81 = x_8.x_GLF_uniform_int_values[1].el;
    int x_84 = x_8.x_GLF_uniform_int_values[0].el;
    int x_87 = x_8.x_GLF_uniform_int_values[0].el;
    int x_90 = x_8.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(float(x_81), float(x_84), float(x_87), float(x_90));
  } else {
    int x_94 = x_8.x_GLF_uniform_int_values[0].el;
    float x_95 = float(x_94);
    x_GLF_color = vec4(x_95, x_95, x_95, x_95);
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
ERROR: 0:39: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' global bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:39: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



