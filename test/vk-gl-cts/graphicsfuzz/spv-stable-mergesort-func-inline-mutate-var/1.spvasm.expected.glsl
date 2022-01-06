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
  int x_303 = from;
  k = x_303;
  int x_304 = from;
  i = x_304;
  int x_305 = mid;
  j = (x_305 + 1);
  while (true) {
    int x_311 = i;
    int x_312 = mid;
    int x_314 = j;
    int x_315 = to;
    if (((x_311 <= x_312) & (x_314 <= x_315))) {
    } else {
      break;
    }
    int x_321 = data[i];
    int x_324 = data[j];
    if ((x_321 < x_324)) {
      int x_329 = k;
      k = (x_329 + 1);
      int x_331 = i;
      i = (x_331 + 1);
      int x_334 = data[x_331];
      temp[x_329] = x_334;
    } else {
      int x_336 = k;
      k = (x_336 + 1);
      int x_338 = j;
      j = (x_338 + 1);
      int x_341 = data[x_338];
      temp[x_336] = x_341;
    }
  }
  while (true) {
    int x_347 = i;
    int x_349 = i;
    int x_350 = mid;
    if (((x_347 < 10) & (x_349 <= x_350))) {
    } else {
      break;
    }
    int x_354 = k;
    k = (x_354 + 1);
    int x_356 = i;
    i = (x_356 + 1);
    int x_359 = data[x_356];
    temp[x_354] = x_359;
  }
  int x_361 = from;
  i_1 = x_361;
  while (true) {
    int x_366 = i_1;
    int x_367 = to;
    if ((x_366 <= x_367)) {
    } else {
      break;
    }
    int x_370 = i_1;
    int x_373 = temp[i_1];
    data[x_370] = x_373;
    {
      i_1 = (i_1 + 1);
    }
  }
  return;
}

void main_1() {
  int x_85 = 0;
  int x_86 = 0;
  int x_87 = 0;
  int x_88 = 0;
  int x_89 = 0;
  int x_90 = 0;
  int x_91 = 0;
  int x_92 = 0;
  int x_93 = 0;
  int x_94 = 0;
  int i_3 = 0;
  int j_1 = 0;
  float grey = 0.0f;
  float x_96 = x_28.injectionSwitch.x;
  i_3 = int(x_96);
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
      int x_145 = j_1;
      int x_148 = data[j_1];
      temp[x_145] = x_148;
    }
  }
  x_94 = 0;
  x_93 = 9;
  x_92 = 1;
  {
    for(; (x_92 <= x_93); x_92 = (2 * x_92)) {
      x_91 = x_94;
      {
        for(; (x_91 < x_93); x_91 = (x_91 + (2 * x_92))) {
          x_90 = x_91;
          int x_170 = x_91;
          int x_171 = x_92;
          int x_173[10] = data;
          int tint_symbol_6[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          data = tint_symbol_6;
          data = x_173;
          x_89 = ((x_170 + x_171) - 1);
          x_88 = min(((x_91 + (2 * x_92)) - 1), x_93);
          x_87 = x_90;
          x_86 = x_89;
          x_85 = x_88;
          merge_i1_i1_i1_(x_87, x_86, x_85);
        }
      }
    }
  }
  float x_194 = tint_symbol.y;
  if ((int(x_194) < 30)) {
    int x_201 = data[0];
    grey = (0.5f + (float(x_201) / 10.0f));
  } else {
    float x_206 = tint_symbol.y;
    if ((int(x_206) < 60)) {
      int x_213 = data[1];
      grey = (0.5f + (float(x_213) / 10.0f));
    } else {
      float x_218 = tint_symbol.y;
      if ((int(x_218) < 90)) {
        int x_225 = data[2];
        grey = (0.5f + (float(x_225) / 10.0f));
      } else {
        float x_230 = tint_symbol.y;
        if ((int(x_230) < 120)) {
          int x_237 = data[3];
          grey = (0.5f + (float(x_237) / 10.0f));
        } else {
          float x_242 = tint_symbol.y;
          if ((int(x_242) < 150)) {
            discard;
          } else {
            float x_249 = tint_symbol.y;
            if ((int(x_249) < 180)) {
              int x_256 = data[5];
              grey = (0.5f + (float(x_256) / 10.0f));
            } else {
              float x_261 = tint_symbol.y;
              if ((int(x_261) < 210)) {
                int x_268 = data[6];
                grey = (0.5f + (float(x_268) / 10.0f));
              } else {
                float x_273 = tint_symbol.y;
                if ((int(x_273) < 240)) {
                  int x_280 = data[7];
                  grey = (0.5f + (float(x_280) / 10.0f));
                } else {
                  float x_285 = tint_symbol.y;
                  if ((int(x_285) < 270)) {
                    int x_292 = data[8];
                    grey = (0.5f + (float(x_292) / 10.0f));
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
  float x_296 = grey;
  vec3 x_297 = vec3(x_296, x_296, x_296);
  x_GLF_color = vec4(x_297.x, x_297.y, x_297.z, 1.0f);
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
  main_out tint_symbol_7 = main_out(x_GLF_color);
  return tint_symbol_7;
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



