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
  float x_146 = tint_symbol.y;
  float x_148 = x_9.resolution.y;
  if ((x_146 < (x_148 / 2.0f))) {
    float x_154 = a;
    float x_155 = b;
    x_144 = (x_154 > x_155);
  } else {
    float x_157 = a;
    float x_158 = b;
    x_144 = (x_157 < x_158);
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
      int x_56 = i;
      if ((x_56 < 10)) {
      } else {
        break;
      }
      int x_59 = i;
      int x_60 = i;
      float x_64 = x_13.injectionSwitch.y;
      data[x_59] = (float((10 - x_60)) * x_64);
      {
        int x_67 = i;
        i = (x_67 + 1);
      }
      continue;
    }
  }
  i_1 = 0;
  {
    while(true) {
      int x_73 = i_1;
      if ((x_73 < 9)) {
      } else {
        break;
      }
      j = 0;
      {
        while(true) {
          int x_80 = j;
          if ((x_80 < 10)) {
          } else {
            break;
          }
          int x_83 = j;
          int x_84 = i_1;
          if ((x_83 < (x_84 + 1))) {
            {
              int x_110 = j;
              j = (x_110 + 1);
            }
            continue;
          }
          int x_89 = i_1;
          int x_90 = j;
          float x_92 = data[x_89];
          param = x_92;
          float x_94 = data[x_90];
          param_1 = x_94;
          bool x_95 = checkSwap_f1_f1_(param, param_1);
          doSwap = x_95;
          bool x_96 = doSwap;
          if (x_96) {
            int x_99 = i_1;
            float x_101 = data[x_99];
            temp = x_101;
            int x_102 = i_1;
            int x_103 = j;
            float x_105 = data[x_103];
            data[x_102] = x_105;
            int x_107 = j;
            float x_108 = temp;
            data[x_107] = x_108;
          }
          {
            int x_110 = j;
            j = (x_110 + 1);
          }
          continue;
        }
      }
      {
        int x_112 = i_1;
        i_1 = (x_112 + 1);
      }
      continue;
    }
  }
  float x_115 = tint_symbol.x;
  float x_117 = x_9.resolution.x;
  if ((x_115 < (x_117 / 2.0f))) {
    float x_124 = data[0];
    float x_127 = data[5];
    float x_130 = data[9];
    x_GLF_color = vec4((x_124 / 10.0f), (x_127 / 10.0f), (x_130 / 10.0f), 1.0f);
  } else {
    float x_134 = data[5];
    float x_137 = data[9];
    float x_140 = data[0];
    x_GLF_color = vec4((x_134 / 10.0f), (x_137 / 10.0f), (x_140 / 10.0f), 1.0f);
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
