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
  int x_262 = from;
  k = x_262;
  int x_263 = from;
  i = x_263;
  int x_264 = mid;
  j = (x_264 + 1);
  while (true) {
    int x_270 = i;
    int x_271 = mid;
    int x_273 = j;
    int x_274 = to;
    if (((x_270 <= x_271) & (x_273 <= x_274))) {
    } else {
      break;
    }
    int x_280 = data[i];
    int x_283 = data[j];
    if ((x_280 < x_283)) {
      int x_288 = k;
      k = (x_288 + 1);
      int x_290 = i;
      i = (x_290 + 1);
      int x_293 = data[x_290];
      temp[x_288] = x_293;
    } else {
      int x_295 = k;
      k = (x_295 + 1);
      int x_297 = j;
      j = (x_297 + 1);
      int x_300 = data[x_297];
      temp[x_295] = x_300;
    }
  }
  while (true) {
    int x_306 = i;
    int x_308 = i;
    int x_309 = mid;
    if (((x_306 < 10) & (x_308 <= x_309))) {
    } else {
      break;
    }
    int x_313 = k;
    k = (x_313 + 1);
    int x_315 = i;
    i = (x_315 + 1);
    int x_318 = data[x_315];
    temp[x_313] = x_318;
  }
  int x_320 = from;
  i_1 = x_320;
  while (true) {
    int x_325 = i_1;
    int x_326 = to;
    if ((x_325 <= x_326)) {
    } else {
      break;
    }
    int x_329 = i_1;
    int x_332 = temp[i_1];
    data[x_329] = x_332;
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
  float x_86 = x_28.injectionSwitch.x;
  i_3 = int(x_86);
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
  while (true) {
    bool x_133 = (j_1 < 10);
    float x_135 = x_28.injectionSwitch.x;
    if (!((x_135 <= 1.0f))) {
      grey = 1.0f;
    }
    if (x_133) {
    } else {
      break;
    }
    int x_140 = j_1;
    int x_143 = data[j_1];
    temp[x_140] = x_143;
    {
      j_1 = (j_1 + 1);
    }
  }
  mergeSort_();
  float x_149 = tint_symbol.y;
  if ((int(x_149) < 30)) {
    int x_156 = data[0];
    grey = (0.5f + (float(x_156) / 10.0f));
  } else {
    float x_161 = tint_symbol.y;
    if ((int(x_161) < 60)) {
      int x_168 = data[1];
      grey = (0.5f + (float(x_168) / 10.0f));
    } else {
      float x_173 = tint_symbol.y;
      if ((int(x_173) < 90)) {
        int x_180 = data[2];
        grey = (0.5f + (float(x_180) / 10.0f));
      } else {
        float x_185 = tint_symbol.y;
        if ((int(x_185) < 120)) {
          int x_192 = data[3];
          grey = (0.5f + (float(x_192) / 10.0f));
        } else {
          float x_197 = tint_symbol.y;
          if ((int(x_197) < 150)) {
            discard;
          } else {
            float x_204 = tint_symbol.y;
            if ((int(x_204) < 180)) {
              int x_211 = data[5];
              grey = (0.5f + (float(x_211) / 10.0f));
            } else {
              float x_216 = tint_symbol.y;
              if ((int(x_216) < 210)) {
                int x_223 = data[6];
                grey = (0.5f + (float(x_223) / 10.0f));
              } else {
                float x_228 = tint_symbol.y;
                if ((int(x_228) < 240)) {
                  int x_235 = data[7];
                  grey = (0.5f + (float(x_235) / 10.0f));
                } else {
                  float x_240 = tint_symbol.y;
                  bool guard233 = true;
                  if ((int(x_240) < 270)) {
                    int x_247 = data[8];
                    grey = (0.5f + (float(x_247) / 10.0f));
                    guard233 = false;
                  } else {
                    if (guard233) {
                      float x_252 = x_28.injectionSwitch.y;
                      if (!((0.0f < x_252))) {
                        guard233 = false;
                      }
                      if (guard233) {
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
    }
  }
  float x_255 = grey;
  vec3 x_256 = vec3(x_255, x_255, x_255);
  x_GLF_color = vec4(x_256.x, x_256.y, x_256.z, 1.0f);
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



