SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 injectionSwitch;
};

layout (binding = 0) uniform buf0_1 {
  vec2 injectionSwitch;
} x_25;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

vec3 drawShape_vf2_(inout vec2 pos) {
  bool c2 = false;
  bool c3 = false;
  bool c4 = false;
  bool c5 = false;
  bool c6 = false;
  int GLF_live4i = 0;
  int GLF_live4_looplimiter5 = 0;
  mat4x2 GLF_live7m42 = mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  mat3 GLF_live7m33 = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int GLF_live7cols = 0;
  int GLF_live7_looplimiter3 = 0;
  int GLF_live7rows = 0;
  int GLF_live7_looplimiter2 = 0;
  int GLF_live7_looplimiter1 = 0;
  int GLF_live7c = 0;
  int GLF_live7r = 0;
  int GLF_live7_looplimiter0 = 0;
  int GLF_live7sum_index = 0;
  int GLF_live7_looplimiter7 = 0;
  int GLF_live7cols_1 = 0;
  int GLF_live7rows_1 = 0;
  float GLF_live7sums[9] = float[9](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int GLF_live7c_1 = 0;
  int GLF_live7r_1 = 0;
  int x_180 = 0;
  mat3 indexable = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float x_182 = pos.x;
  c2 = (x_182 > 1.0f);
  if (c2) {
    return vec3(1.0f, 1.0f, 1.0f);
  }
  float x_188 = pos.y;
  c3 = (x_188 < 1.0f);
  if (c3) {
    return vec3(1.0f, 1.0f, 1.0f);
  }
  float x_194 = pos.y;
  c4 = (x_194 > 1.0f);
  if (c4) {
    return vec3(1.0f, 1.0f, 1.0f);
  }
  float x_200 = pos.x;
  c5 = (x_200 < 1.0f);
  if (c5) {
    return vec3(1.0f, 1.0f, 1.0f);
  }
  float x_206 = pos.x;
  c6 = ((x_206 + 1.0f) > 1.0f);
  if (c6) {
    return vec3(1.0f, 1.0f, 1.0f);
  }
  GLF_live4i = 0;
  {
    for(; (GLF_live4i < 4); GLF_live4i = (GLF_live4i + 1)) {
      if ((GLF_live4_looplimiter5 >= 7)) {
        break;
      }
      GLF_live4_looplimiter5 = (GLF_live4_looplimiter5 + 1);
      GLF_live7m42 = mat4x2(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(0.0f, 0.0f), vec2(1.0f, 0.0f));
      GLF_live7m33 = mat3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
      GLF_live7cols = 2;
      {
        for(; (GLF_live7cols < 4); GLF_live7cols = (GLF_live7cols + 1)) {
          if ((GLF_live7_looplimiter3 >= 7)) {
            break;
          }
          GLF_live7_looplimiter3 = (GLF_live7_looplimiter3 + 1);
          GLF_live7rows = 2;
          {
            for(; (GLF_live7rows < 4); GLF_live7rows = (GLF_live7rows + 1)) {
              if ((GLF_live7_looplimiter2 >= 7)) {
                break;
              }
              GLF_live7_looplimiter2 = (GLF_live7_looplimiter2 + 1);
              GLF_live7_looplimiter1 = 0;
              GLF_live7c = 0;
              {
                for(; (GLF_live7c < 3); GLF_live7c = (GLF_live7c + 1)) {
                  if ((GLF_live7_looplimiter1 >= 7)) {
                    break;
                  }
                  GLF_live7_looplimiter1 = (GLF_live7_looplimiter1 + 1);
                  GLF_live7r = 0;
                  {
                    for(; (GLF_live7r < 2); GLF_live7r = (GLF_live7r + 1)) {
                      if ((GLF_live7_looplimiter0 >= 7)) {
                        break;
                      }
                      GLF_live7_looplimiter0 = (GLF_live7_looplimiter0 + 1);
                      GLF_live7m33[(((GLF_live7c >= 0) & (GLF_live7c < 3)) ? GLF_live7c : 0)][(((GLF_live7r >= 0) & (GLF_live7r < 3)) ? GLF_live7r : 0)] = 1.0f;
                      float x_267 = x_25.injectionSwitch.y;
                      if ((0.0f > x_267)) {
                      } else {
                        GLF_live7m42[(((GLF_live7c >= 0) & (GLF_live7c < 4)) ? GLF_live7c : 0)][(((GLF_live7r >= 0) & (GLF_live7r < 2)) ? GLF_live7r : 0)] = 1.0f;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      GLF_live7sum_index = 0;
      GLF_live7_looplimiter7 = 0;
      GLF_live7cols_1 = 2;
      {
        for(; (GLF_live7cols_1 < 4); GLF_live7cols_1 = (GLF_live7cols_1 + 1)) {
          if ((GLF_live7_looplimiter7 >= 7)) {
            break;
          }
          GLF_live7_looplimiter7 = (GLF_live7_looplimiter7 + 1);
          GLF_live7rows_1 = 2;
          GLF_live7sums[(((GLF_live7sum_index >= 0) & (GLF_live7sum_index < 9)) ? GLF_live7sum_index : 0)] = 0.0f;
          GLF_live7c_1 = 0;
          {
            for(; (GLF_live7c_1 < 1); GLF_live7c_1 = (GLF_live7c_1 + 1)) {
              GLF_live7r_1 = 0;
              {
                for(; (GLF_live7r_1 < GLF_live7rows_1); GLF_live7r_1 = (GLF_live7r_1 + 1)) {
                  int x_310 = (((GLF_live7sum_index >= 0) & (GLF_live7sum_index < 9)) ? GLF_live7sum_index : 0);
                  mat3 x_312 = transpose(GLF_live7m33);
                  if ((GLF_live7c_1 < 3)) {
                    x_180 = 1;
                  } else {
                    float x_318 = x_25.injectionSwitch.x;
                    x_180 = int(x_318);
                  }
                  int x_320 = x_180;
                  int x_93 = GLF_live7r_1;
                  indexable = x_312;
                  float x_324 = indexable[x_320][((x_93 < 3) ? 1 : 0)];
                  float x_326 = GLF_live7sums[x_310];
                  GLF_live7sums[x_310] = (x_326 + x_324);
                  int x_332 = (((GLF_live7sum_index >= 0) & (GLF_live7sum_index < 9)) ? GLF_live7sum_index : 0);
                  float x_334 = GLF_live7m42[1][GLF_live7r_1];
                  float x_336 = GLF_live7sums[x_332];
                  GLF_live7sums[x_332] = (x_336 + x_334);
                }
              }
            }
          }
          GLF_live7sum_index = (GLF_live7sum_index + 1);
        }
      }
    }
  }
  return vec3(1.0f, 1.0f, 1.0f);
}

void main_1() {
  vec2 position = vec2(0.0f, 0.0f);
  vec2 param = vec2(0.0f, 0.0f);
  vec2 param_1 = vec2(0.0f, 0.0f);
  int i = 0;
  vec2 param_2 = vec2(0.0f, 0.0f);
  float x_161 = x_25.injectionSwitch.x;
  if ((x_161 >= 2.0f)) {
    vec4 x_165 = tint_symbol;
    position = vec2(x_165.x, x_165.y);
    param = position;
    vec3 x_168 = drawShape_vf2_(param);
    param_1 = position;
    vec3 x_170 = drawShape_vf2_(param_1);
    i = 25;
    {
      for(; (i > 0); i = (i - 1)) {
        param_2 = position;
        vec3 x_178 = drawShape_vf2_(param_2);
      }
    }
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
ERROR: 0:104: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:104: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



