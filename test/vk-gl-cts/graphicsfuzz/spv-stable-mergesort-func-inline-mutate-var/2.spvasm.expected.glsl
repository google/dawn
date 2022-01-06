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
  int x_302 = from;
  k = x_302;
  int x_303 = from;
  i = x_303;
  int x_304 = mid;
  j = (x_304 + 1);
  while (true) {
    int x_310 = i;
    int x_311 = mid;
    int x_313 = j;
    int x_314 = to;
    if (((x_310 <= x_311) & (x_313 <= x_314))) {
    } else {
      break;
    }
    int x_320 = data[i];
    int x_323 = data[j];
    if ((x_320 < x_323)) {
      int x_328 = k;
      k = (x_328 + 1);
      int x_330 = i;
      i = (x_330 + 1);
      int x_333 = data[x_330];
      temp[x_328] = x_333;
    } else {
      int x_335 = k;
      k = (x_335 + 1);
      int x_337 = j;
      j = (x_337 + 1);
      int x_340 = data[x_337];
      temp[x_335] = x_340;
    }
  }
  while (true) {
    int x_346 = i;
    int x_348 = i;
    int x_349 = mid;
    if (((x_346 < 10) & (x_348 <= x_349))) {
    } else {
      break;
    }
    int x_353 = k;
    k = (x_353 + 1);
    int x_355 = i;
    i = (x_355 + 1);
    int x_358 = data[x_355];
    temp[x_353] = x_358;
  }
  int x_360 = from;
  i_1 = x_360;
  while (true) {
    int x_365 = i_1;
    int x_366 = to;
    if ((x_365 <= x_366)) {
    } else {
      break;
    }
    int x_369 = i_1;
    int x_372 = temp[i_1];
    data[x_369] = x_372;
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
          x_89 = ((x_91 + x_92) - 1);
          x_88 = min(((x_91 + (2 * x_92)) - 1), x_93);
          x_87 = x_90;
          x_86 = x_89;
          x_85 = x_88;
          merge_i1_i1_i1_(x_87, x_86, x_85);
        }
      }
    }
  }
  float x_193 = tint_symbol.y;
  if ((int(x_193) < 30)) {
    int x_200 = data[0];
    grey = (0.5f + (float(x_200) / 10.0f));
  } else {
    float x_205 = tint_symbol.y;
    if ((int(x_205) < 60)) {
      int x_212 = data[1];
      grey = (0.5f + (float(x_212) / 10.0f));
    } else {
      float x_217 = tint_symbol.y;
      if ((int(x_217) < 90)) {
        int x_224 = data[2];
        grey = (0.5f + (float(x_224) / 10.0f));
      } else {
        float x_229 = tint_symbol.y;
        if ((int(x_229) < 120)) {
          int x_236 = data[3];
          grey = (0.5f + (float(x_236) / 10.0f));
        } else {
          float x_241 = tint_symbol.y;
          if ((int(x_241) < 150)) {
            discard;
          } else {
            float x_248 = tint_symbol.y;
            if ((int(x_248) < 180)) {
              int x_255 = data[5];
              grey = (0.5f + (float(x_255) / 10.0f));
            } else {
              float x_260 = tint_symbol.y;
              if ((int(x_260) < 210)) {
                int x_267 = data[6];
                grey = (0.5f + (float(x_267) / 10.0f));
              } else {
                float x_272 = tint_symbol.y;
                if ((int(x_272) < 240)) {
                  int x_279 = data[7];
                  grey = (0.5f + (float(x_279) / 10.0f));
                } else {
                  float x_284 = tint_symbol.y;
                  if ((int(x_284) < 270)) {
                    int x_291 = data[8];
                    grey = (0.5f + (float(x_291) / 10.0f));
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
  float x_295 = grey;
  vec3 x_296 = vec3(x_295, x_295, x_295);
  x_GLF_color = vec4(x_296.x, x_296.y, x_296.z, 1.0f);
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



