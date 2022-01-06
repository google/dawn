SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[4];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[4];
} x_5;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int x_28 = 0;
  int x_29 = 0;
  int x_28_phi = 0;
  int x_31_phi = 0;
  int x_42_phi = 0;
  int x_24 = min(1, reversebits(1));
  int x_26 = x_5.x_GLF_uniform_int_values[3].el;
  x_28_phi = x_26;
  x_31_phi = 1;
  while (true) {
    int x_32 = 0;
    x_28 = x_28_phi;
    int x_31 = x_31_phi;
    x_42_phi = x_28;
    if ((x_31 <= (x_24 - 1))) {
    } else {
      break;
    }
    x_29 = (x_28 + x_31);
    int x_38 = x_5.x_GLF_uniform_int_values[0].el;
    if ((x_38 == 1)) {
      x_42_phi = x_29;
      break;
    }
    {
      x_32 = (x_31 + 1);
      x_28_phi = x_29;
      x_31_phi = x_32;
    }
  }
  int x_42 = x_42_phi;
  int x_44 = x_5.x_GLF_uniform_int_values[2].el;
  if ((x_42 == x_44)) {
    int x_50 = x_5.x_GLF_uniform_int_values[0].el;
    float x_51 = float(x_50);
    int x_53 = x_5.x_GLF_uniform_int_values[1].el;
    float x_54 = float(x_53);
    x_GLF_color = vec4(x_51, x_54, x_54, x_51);
  } else {
    int x_57 = x_5.x_GLF_uniform_int_values[1].el;
    float x_58 = float(x_57);
    x_GLF_color = vec4(x_58, x_58, x_58, x_58);
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
ERROR: 0:22: 'reversebits' : no matching overloaded function found 
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



