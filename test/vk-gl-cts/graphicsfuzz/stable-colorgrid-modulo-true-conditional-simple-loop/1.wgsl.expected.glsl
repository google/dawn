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
layout (binding = 1) uniform buf1_1 {
  vec2 injectionSwitch;
} x_19;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

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
        int x_155 = i;
        float x_157 = thirty_two;
        if (((float(x_155) % round(x_157)) <= 0.01f)) {
          result = (result + 100.0f);
        }
      }
      int x_165 = i;
      float x_167 = limit;
      if ((float(x_165) >= x_167)) {
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
  vec3 x_61 = vec3(0.0f, 0.0f, 0.0f);
  int i_1 = 0;
  float j = 0.0f;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_63 = x_13.resolution.x;
  thirty_two_1 = round((x_63 / 8.0f));
  float x_67 = tint_symbol.x;
  param = x_67;
  param_1 = thirty_two_1;
  float x_69 = compute_value_f1_f1_(param, param_1);
  c.x = x_69;
  float x_72 = tint_symbol.y;
  param_2 = x_72;
  param_3 = thirty_two_1;
  float x_74 = compute_value_f1_f1_(param_2, param_3);
  c.y = x_74;
  float x_77 = c.x;
  if (true) {
    x_61 = c;
  } else {
    vec3 x_82 = c;
    float x_84 = x_19.injectionSwitch.x;
    x_61 = (x_82 * x_84);
  }
  float x_87 = x_61.y;
  c.z = (x_77 + x_87);
  i_1 = 0;
  {
    for(; (i_1 < 3); i_1 = (i_1 + 1)) {
      float x_99 = c[i_1];
      if ((x_99 >= 1.0f)) {
        int x_103 = i_1;
        float x_106 = c[i_1];
        float x_109 = c[i_1];
        c[x_103] = (x_106 * x_109);
      }
      j = 0.0f;
      while (true) {
        float x_117 = x_19.injectionSwitch.x;
        float x_119 = x_19.injectionSwitch.y;
        if ((x_117 > x_119)) {
        } else {
          break;
        }
        float x_122 = j;
        float x_124 = x_19.injectionSwitch.x;
        if ((x_122 >= x_124)) {
          break;
        }
        j = (j + 1.0f);
      }
    }
  }
  vec3 x_134 = normalize(abs(c));
  x_GLF_color = vec4(x_134.x, x_134.y, x_134.z, 1.0f);
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



