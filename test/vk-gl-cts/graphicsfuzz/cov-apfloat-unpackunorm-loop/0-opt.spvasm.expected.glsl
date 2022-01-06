SKIP: FAILED

#version 310 es
precision mediump float;

vec4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}


struct tint_padded_array_element {
  int el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[3];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[3];
} x_6;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  vec4 v = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  int x_30 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_30;
  while (true) {
    int x_35 = i;
    int x_37 = x_6.x_GLF_uniform_int_values[2].el;
    if ((x_35 < x_37)) {
    } else {
      break;
    }
    v = tint_unpack4x8unorm(100u);
    float x_42 = v.x;
    if ((int(x_42) > i)) {
      int x_49 = x_6.x_GLF_uniform_int_values[1].el;
      float x_50 = float(x_49);
      x_GLF_color = vec4(x_50, x_50, x_50, x_50);
      return;
    }
    {
      i = (i + 1);
    }
  }
  int x_55 = x_6.x_GLF_uniform_int_values[0].el;
  int x_58 = x_6.x_GLF_uniform_int_values[1].el;
  int x_61 = x_6.x_GLF_uniform_int_values[1].el;
  int x_64 = x_6.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4(float(x_55), float(x_58), float(x_61), float(x_64));
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



