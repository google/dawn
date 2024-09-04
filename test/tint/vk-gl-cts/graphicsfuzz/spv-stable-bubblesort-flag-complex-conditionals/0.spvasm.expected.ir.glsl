SKIP: FAILED

#version 310 es

struct buf1 {
  vec2 resolution;
};

struct buf0 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_9;
uniform buf0 x_13;
vec4 x_GLF_color = vec4(0.0f);
bool checkSwap_f1_f1_(inout float a, inout float b) {
  bool x_144 = false;
  if ((tint_symbol.y < (x_9.resolution.y / 2.0f))) {
    x_144 = (a > b);
  } else {
    x_144 = (a < b);
  }
  bool x_160 = x_144;
  return x_160;
}
void main_1() {
  int i = 0;
  float data[10] = float[10](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int i_1 = 0;
  int j = 0;
  bool doSwap = false;
  float param = 0.0f;
  float param_1 = 0.0f;
  float temp = 0.0f;
  i = 0;
  {
    while(true) {
      if ((i < 10)) {
      } else {
        break;
      }
      int x_59 = i;
      float v = float((10 - i));
      data[x_59] = (v * x_13.injectionSwitch.y);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  i_1 = 0;
  {
    while(true) {
      if ((i_1 < 9)) {
      } else {
        break;
      }
      j = 0;
      {
        while(true) {
          if ((j < 10)) {
          } else {
            break;
          }
          if ((j < (i_1 + 1))) {
            {
              j = (j + 1);
            }
            continue;
          }
          int x_90 = j;
          param = data[i_1];
          param_1 = data[x_90];
          bool x_95 = checkSwap_f1_f1_(param, param_1);
          doSwap = x_95;
          if (doSwap) {
            temp = data[i_1];
            int x_102 = i_1;
            data[x_102] = data[j];
            int x_107 = j;
            data[x_107] = temp;
          }
          {
            j = (j + 1);
          }
          continue;
        }
      }
      {
        i_1 = (i_1 + 1);
      }
      continue;
    }
  }
  if ((tint_symbol.x < (x_9.resolution.x / 2.0f))) {
    x_GLF_color = vec4((data[0] / 10.0f), (data[5] / 10.0f), (data[9] / 10.0f), 1.0f);
  } else {
    x_GLF_color = vec4((data[5] / 10.0f), (data[9] / 10.0f), (data[0] / 10.0f), 1.0f);
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
