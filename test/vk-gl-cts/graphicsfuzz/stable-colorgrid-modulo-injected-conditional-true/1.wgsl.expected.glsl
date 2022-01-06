SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 resolution;
};
struct buf1 {
  vec2 injectionSwitch;
};

layout (binding = 0) uniform buf0_1 {
  vec2 resolution;
} x_13;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 1) uniform buf1_1 {
  vec2 injectionSwitch;
} x_20;

float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float result = 0.0f;
  int i = 0;
  result = -0.5f;
  i = 1;
  {
    for(; (i < 800); i = (i + 1)) {
      if (((i % 32) == 0)) {
        result = (result + 0.400000006f);
      } else {
        int x_136 = i;
        float x_138 = thirty_two;
        if (((float(x_136) % round(x_138)) <= 0.01f)) {
          result = (result + 100.0f);
        }
      }
      int x_146 = i;
      float x_148 = limit;
      if ((float(x_146) >= x_148)) {
        return result;
      }
    }
  }
  return result;
}

void main_1() {
  vec3 c = vec3(0.0f, 0.0f, 0.0f);
  float thirty_two_1 = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  int i_1 = 0;
  vec3 x_58 = vec3(0.0f, 0.0f, 0.0f);
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_60 = x_13.resolution.x;
  thirty_two_1 = round((x_60 / 8.0f));
  float x_64 = tint_symbol.x;
  param = x_64;
  param_1 = thirty_two_1;
  float x_66 = compute_value_f1_f1_(param, param_1);
  c.x = x_66;
  float x_69 = tint_symbol.y;
  param_2 = x_69;
  param_3 = thirty_two_1;
  float x_71 = compute_value_f1_f1_(param_2, param_3);
  c.y = x_71;
  float x_74 = c.x;
  float x_76 = c.y;
  c.z = (x_74 + x_76);
  i_1 = 0;
  {
    for(; (i_1 < 3); i_1 = (i_1 + 1)) {
      float x_88 = c[i_1];
      if ((x_88 >= 1.0f)) {
        int x_92 = i_1;
        float x_95 = c[i_1];
        float x_98 = c[i_1];
        c[x_92] = (x_95 * x_98);
      }
    }
  }
  float x_104 = x_20.injectionSwitch.x;
  float x_106 = x_20.injectionSwitch.y;
  if ((x_104 < x_106)) {
    x_58 = abs(c);
  } else {
    x_58 = c;
  }
  vec3 x_115 = normalize(x_58);
  x_GLF_color = vec4(x_115.x, x_115.y, x_115.z, 1.0f);
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_4 {
  vec4 tint_symbol_2;
};
struct tint_symbol_5 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out tint_symbol_6 = main_out(x_GLF_color);
  return tint_symbol_6;
}

tint_symbol_5 tint_symbol_1(tint_symbol_4 tint_symbol_3) {
  main_out inner_result = tint_symbol_1_inner(tint_symbol_3.tint_symbol_2);
  tint_symbol_5 wrapper_result = tint_symbol_5(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_4 inputs;
  inputs.tint_symbol_2 = gl_FragCoord;
  tint_symbol_5 outputs;
  outputs = tint_symbol_1(inputs);
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:32: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' temp float' and a right operand of type ' global mediump float' (or there is no acceptable conversion)
ERROR: 0:32: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



