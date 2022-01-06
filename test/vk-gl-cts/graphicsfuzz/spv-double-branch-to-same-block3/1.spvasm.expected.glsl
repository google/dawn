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
  int x_256 = from;
  k = x_256;
  int x_257 = from;
  i = x_257;
  int x_258 = mid;
  j = (x_258 + 1);
  while (true) {
    int x_264 = i;
    int x_265 = mid;
    int x_267 = j;
    int x_268 = to;
    if (((x_264 <= x_265) & (x_267 <= x_268))) {
    } else {
      break;
    }
    int x_274 = data[i];
    int x_277 = data[j];
    if ((x_274 < x_277)) {
      int x_282 = k;
      k = (x_282 + 1);
      int x_284 = i;
      i = (x_284 + 1);
      int x_287 = data[x_284];
      temp[x_282] = x_287;
    } else {
      int x_289 = k;
      k = (x_289 + 1);
      int x_291 = j;
      j = (x_291 + 1);
      int x_294 = data[x_291];
      temp[x_289] = x_294;
    }
  }
  while (true) {
    if (!((256.0f < 1.0f))) {
    } else {
      continue;
    }
    int x_301 = i;
    int x_303 = i;
    int x_304 = mid;
    if (((x_301 < 10) & (x_303 <= x_304))) {
    } else {
      break;
    }
    int x_309 = k;
    k = (x_309 + 1);
    int x_311 = i;
    i = (x_311 + 1);
    int x_314 = data[x_311];
    temp[x_309] = x_314;
  }
  int x_316 = from;
  i_1 = x_316;
  while (true) {
    int x_321 = i_1;
    int x_322 = to;
    if ((x_321 <= x_322)) {
    } else {
      break;
    }
    int x_325 = i_1;
    int x_328 = temp[i_1];
    data[x_325] = x_328;
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
  float x_89 = x_28.injectionSwitch.x;
  i_3 = int(x_89);
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
      int x_138 = j_1;
      int x_141 = data[j_1];
      temp[x_138] = x_141;
    }
  }
  mergeSort_();
  float x_147 = tint_symbol.y;
  if ((int(x_147) < 30)) {
    int x_154 = data[0];
    grey = (0.5f + (float(x_154) / 10.0f));
  } else {
    float x_159 = tint_symbol.y;
    if ((int(x_159) < 60)) {
      int x_166 = data[1];
      grey = (0.5f + (float(x_166) / 10.0f));
    } else {
      float x_171 = tint_symbol.y;
      if ((int(x_171) < 90)) {
        int x_178 = data[2];
        grey = (0.5f + (float(x_178) / 10.0f));
      } else {
        float x_183 = tint_symbol.y;
        if ((int(x_183) < 120)) {
          int x_190 = data[3];
          grey = (0.5f + (float(x_190) / 10.0f));
        } else {
          float x_195 = tint_symbol.y;
          if ((int(x_195) < 150)) {
            discard;
          } else {
            float x_202 = tint_symbol.y;
            if ((int(x_202) < 180)) {
              int x_209 = data[5];
              grey = (0.5f + (float(x_209) / 10.0f));
            } else {
              float x_214 = tint_symbol.y;
              if ((int(x_214) < 210)) {
                int x_221 = data[6];
                grey = (0.5f + (float(x_221) / 10.0f));
              } else {
                float x_226 = tint_symbol.y;
                if ((int(x_226) < 240)) {
                  int x_233 = data[7];
                  grey = (0.5f + (float(x_233) / 10.0f));
                } else {
                  float x_238 = tint_symbol.y;
                  if ((int(x_238) < 270)) {
                    int x_245 = data[8];
                    grey = (0.5f + (float(x_245) / 10.0f));
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
  float x_249 = grey;
  vec3 x_250 = vec3(x_249, x_249, x_249);
  x_GLF_color = vec4(x_250.x, x_250.y, x_250.z, 1.0f);
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



