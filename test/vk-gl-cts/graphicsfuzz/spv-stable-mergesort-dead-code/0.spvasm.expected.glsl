SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 injectionSwitch;
};

int data[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
int temp[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
layout (binding = 0) uniform buf0_1 {
  vec2 injectionSwitch;
} x_28;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void merge_i1_i1_i1_(inout int from, inout int mid, inout int to) {
  int k = 0;
  int i = 0;
  int j = 0;
  int i_1 = 0;
  int x_251 = from;
  k = x_251;
  int x_252 = from;
  i = x_252;
  int x_253 = mid;
  j = (x_253 + 1);
  while (true) {
    int x_259 = i;
    int x_260 = mid;
    int x_262 = j;
    int x_263 = to;
    if (((x_259 <= x_260) & (x_262 <= x_263))) {
    } else {
      break;
    }
    int x_269 = data[i];
    int x_272 = data[j];
    if ((x_269 < x_272)) {
      int x_277 = k;
      k = (x_277 + 1);
      int x_279 = i;
      i = (x_279 + 1);
      int x_282 = data[x_279];
      temp[x_277] = x_282;
    } else {
      int x_284 = k;
      k = (x_284 + 1);
      int x_286 = j;
      j = (x_286 + 1);
      int x_289 = data[x_286];
      temp[x_284] = x_289;
    }
  }
  while (true) {
    int x_295 = i;
    int x_297 = i;
    int x_298 = mid;
    if (((x_295 < 10) & (x_297 <= x_298))) {
    } else {
      break;
    }
    int x_302 = k;
    k = (x_302 + 1);
    int x_304 = i;
    i = (x_304 + 1);
    int x_307 = data[x_304];
    temp[x_302] = x_307;
  }
  int x_309 = from;
  i_1 = x_309;
  while (true) {
    int x_314 = i_1;
    int x_315 = to;
    if ((x_314 <= x_315)) {
    } else {
      break;
    }
    int x_318 = i_1;
    int x_321 = temp[i_1];
    data[x_318] = x_321;
    {
      i_1 = (i_1 + 1);
    }
  }
  return;
}

void mergeSort_() {
  int low = 0;
  int high = 0;
  int m = 0;
  int i_2 = 0;
  int from_1 = 0;
  int mid_1 = 0;
  int to_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  low = 0;
  high = 9;
  m = 1;
  {
    for(; (m <= high); m = (2 * m)) {
      i_2 = low;
      {
        for(; (i_2 < high); i_2 = (i_2 + (2 * m))) {
          from_1 = i_2;
          mid_1 = ((i_2 + m) - 1);
          to_1 = min(((i_2 + (2 * m)) - 1), high);
          param = from_1;
          param_1 = mid_1;
          param_2 = to_1;
          merge_i1_i1_i1_(param, param_1, param_2);
        }
      }
    }
  }
  return;
}

void main_1() {
  int i_3 = 0;
  int j_1 = 0;
  float grey = 0.0f;
  float x_84 = x_28.injectionSwitch.x;
  i_3 = int(x_84);
  while (true) {
    switch(i_3) {
      case 9: {
        data[i_3] = -5;
        break;
      }
      case 8: {
        data[i_3] = -4;
        break;
      }
      case 7: {
        data[i_3] = -3;
        break;
      }
      case 6: {
        data[i_3] = -2;
        break;
      }
      case 5: {
        data[i_3] = -1;
        break;
      }
      case 4: {
        data[i_3] = 0;
        break;
      }
      case 3: {
        data[i_3] = 1;
        break;
      }
      case 2: {
        data[i_3] = 2;
        break;
      }
      case 1: {
        data[i_3] = 3;
        break;
      }
      case 0: {
        data[i_3] = 4;
        break;
      }
      default: {
        break;
      }
    }
    i_3 = (i_3 + 1);
    {
      if ((i_3 < 10)) {
      } else {
        break;
      }
    }
  }
  j_1 = 0;
  {
    for(; (j_1 < 10); j_1 = (j_1 + 1)) {
      int x_133 = j_1;
      int x_136 = data[j_1];
      temp[x_133] = x_136;
    }
  }
  mergeSort_();
  float x_142 = tint_symbol.y;
  if ((int(x_142) < 30)) {
    int x_149 = data[0];
    grey = (0.5f + (float(x_149) / 10.0f));
  } else {
    float x_154 = tint_symbol.y;
    if ((int(x_154) < 60)) {
      int x_161 = data[1];
      grey = (0.5f + (float(x_161) / 10.0f));
    } else {
      float x_166 = tint_symbol.y;
      if ((int(x_166) < 90)) {
        int x_173 = data[2];
        grey = (0.5f + (float(x_173) / 10.0f));
      } else {
        float x_178 = tint_symbol.y;
        if ((int(x_178) < 120)) {
          int x_185 = data[3];
          grey = (0.5f + (float(x_185) / 10.0f));
        } else {
          float x_190 = tint_symbol.y;
          if ((int(x_190) < 150)) {
            discard;
          } else {
            float x_197 = tint_symbol.y;
            if ((int(x_197) < 180)) {
              int x_204 = data[5];
              grey = (0.5f + (float(x_204) / 10.0f));
            } else {
              float x_209 = tint_symbol.y;
              if ((int(x_209) < 210)) {
                int x_216 = data[6];
                grey = (0.5f + (float(x_216) / 10.0f));
              } else {
                float x_221 = tint_symbol.y;
                if ((int(x_221) < 240)) {
                  int x_228 = data[7];
                  grey = (0.5f + (float(x_228) / 10.0f));
                } else {
                  float x_233 = tint_symbol.y;
                  if ((int(x_233) < 270)) {
                    int x_240 = data[8];
                    grey = (0.5f + (float(x_240) / 10.0f));
                  } else {
                    discard;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  float x_244 = grey;
  vec3 x_245 = vec3(x_244, x_244, x_244);
  x_GLF_color = vec4(x_245.x, x_245.y, x_245.z, 1.0f);
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
ERROR: 0:32: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:32: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



