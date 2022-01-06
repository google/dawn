SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 resolution;
};

layout (binding = 0) uniform buf0_1 {
  vec2 resolution;
} x_13;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
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
        int x_147 = i;
        float x_149 = thirty_two;
        if (((float(x_147) % round(x_149)) <= 0.01f)) {
          result = (result + 100.0f);
        }
      }
      int x_157 = i;
      float x_159 = limit;
      if ((float(x_157) >= x_159)) {
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
  vec3 x_76 = c;
  vec3 x_79 = c;
  mat4x2 x_87 = mat4x2(vec2(x_79.x, x_79.y), vec2(x_79.z, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 0.0f));
  c.z = ((x_76 * mat3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f))).x + vec3(x_87[0u].x, x_87[0u].y, x_87[1u].x).y);
  i_1 = 0;
  {
    for(; (i_1 < 3); i_1 = (i_1 + 1)) {
      float x_104 = c[i_1];
      if ((x_104 >= 1.0f)) {
        int x_108 = i_1;
        float x_111 = c[i_1];
        float x_114 = c[i_1];
        c[x_108] = (x_111 * x_114);
        float x_118 = tint_symbol.y;
        if ((x_118 < 0.0f)) {
          break;
        }
      }
    }
  }
  vec3 x_126 = normalize(abs(c));
  x_GLF_color = vec4(x_126.x, x_126.y, x_126.z, 1.0f);
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
ERROR: 0:26: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' temp float' and a right operand of type ' global mediump float' (or there is no acceptable conversion)
ERROR: 0:26: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



