SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 injectionSwitch;
};
struct buf1 {
  vec2 resolution;
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
  int x_254 = from;
  k = x_254;
  int x_255 = from;
  i = x_255;
  int x_256 = mid;
  j = (x_256 + 1);
  while (true) {
    int x_262 = i;
    int x_263 = mid;
    int x_265 = j;
    int x_266 = to;
    if (((x_262 <= x_263) & (x_265 <= x_266))) {
    } else {
      break;
    }
    int x_272 = data[i];
    int x_275 = data[j];
    if ((x_272 < x_275)) {
      int x_280 = k;
      k = (x_280 + 1);
      int x_282 = i;
      i = (x_282 + 1);
      int x_285 = data[x_282];
      temp[x_280] = x_285;
    } else {
      int x_287 = k;
      k = (x_287 + 1);
      int x_289 = j;
      j = (x_289 + 1);
      int x_292 = data[x_289];
      temp[x_287] = x_292;
    }
  }
  while (true) {
    int x_298 = i;
    int x_300 = i;
    int x_301 = mid;
    if (((x_298 < 10) & (x_300 <= x_301))) {
    } else {
      break;
    }
    int x_305 = k;
    k = (x_305 + 1);
    int x_307 = i;
    i = (x_307 + 1);
    int x_310 = data[x_307];
    temp[x_305] = x_310;
  }
  int x_312 = from;
  i_1 = x_312;
  while (true) {
    int x_317 = i_1;
    int x_318 = to;
    if ((x_317 <= x_318)) {
    } else {
      break;
    }
    int x_321 = i_1;
    int x_324 = temp[i_1];
    data[x_321] = x_324;
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
  float x_87 = x_28.injectionSwitch.x;
  i_3 = int(x_87);
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
      int x_136 = j_1;
      int x_139 = data[j_1];
      temp[x_136] = x_139;
    }
  }
  mergeSort_();
  float x_145 = tint_symbol.y;
  if ((int(x_145) < 30)) {
    int x_152 = data[0];
    grey = (0.5f + (float(x_152) / 10.0f));
  } else {
    float x_157 = tint_symbol.y;
    if ((int(x_157) < 60)) {
      int x_164 = data[1];
      grey = (0.5f + (float(x_164) / 10.0f));
    } else {
      float x_169 = tint_symbol.y;
      if ((int(x_169) < 90)) {
        int x_176 = data[2];
        grey = (0.5f + (float(x_176) / 10.0f));
      } else {
        float x_181 = tint_symbol.y;
        if ((int(x_181) < 120)) {
          int x_188 = data[3];
          grey = (0.5f + (float(x_188) / 10.0f));
        } else {
          float x_193 = tint_symbol.y;
          if ((int(x_193) < 150)) {
            discard;
          } else {
            float x_200 = tint_symbol.y;
            if ((int(x_200) < 180)) {
              int x_207 = data[5];
              grey = (0.5f + (float(x_207) / 10.0f));
            } else {
              float x_212 = tint_symbol.y;
              if ((int(x_212) < 210)) {
                int x_219 = data[6];
                grey = (0.5f + (float(x_219) / 10.0f));
              } else {
                float x_224 = tint_symbol.y;
                if ((int(x_224) < 240)) {
                  int x_231 = data[7];
                  grey = (0.5f + (float(x_231) / 10.0f));
                } else {
                  float x_236 = tint_symbol.y;
                  if ((int(x_236) < 270)) {
                    int x_243 = data[8];
                    grey = (0.5f + (float(x_243) / 10.0f));
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
  float x_247 = grey;
  vec3 x_248 = vec3(x_247, x_247, x_247);
  x_GLF_color = vec4(x_248.x, x_248.y, x_248.z, 1.0f);
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
ERROR: 0:35: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:35: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



