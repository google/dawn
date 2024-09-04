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
  bool x_147 = false;
  float x_158 = 0.0f;
  float x_159 = 0.0f;
  float x_160 = 0.0f;
  float x_179 = 0.0f;
  float x_178 = 0.0f;
  float x_180 = 0.0f;
  float x_185 = 0.0f;
  float x_184 = 0.0f;
  float x_186 = 0.0f;
  bool x_153 = (tint_symbol.y < (x_9.resolution.y / 2.0f));
  if (x_153) {
    x_158 = a;
    x_160 = x_158;
  } else {
    x_159 = 0.0f;
    x_160 = x_159;
  }
  float x_166 = 0.0f;
  float x_167 = 0.0f;
  float x_168 = 0.0f;
  bool guard155 = true;
  if (false) {
  } else {
    if (guard155) {
      if (x_153) {
        x_166 = b;
        x_168 = x_166;
      } else {
        x_167 = 0.0f;
        x_168 = x_167;
      }
      bool x_169 = (x_160 > x_168);
      if (x_153) {
        x_147 = x_169;
      }
      if (true) {
      } else {
        guard155 = false;
      }
      if (guard155) {
        guard155 = false;
      }
    }
  }
  if (x_153) {
    x_179 = 0.0f;
    x_180 = x_179;
  } else {
    x_178 = a;
    x_180 = x_178;
  }
  if (x_153) {
    x_185 = 0.0f;
    x_186 = x_185;
  } else {
    x_184 = b;
    x_186 = x_184;
  }
  if (x_153) {
  } else {
    x_147 = (x_180 < x_186);
  }
  bool x_191 = x_147;
  return x_191;
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
      int x_62 = i;
      float v = float((10 - i));
      data[x_62] = (v * x_13.injectionSwitch.y);
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
          int x_93 = j;
          param = data[i_1];
          param_1 = data[x_93];
          bool x_98 = checkSwap_f1_f1_(param, param_1);
          doSwap = x_98;
          if (doSwap) {
            temp = data[i_1];
            int x_105 = i_1;
            data[x_105] = data[j];
            int x_110 = j;
            data[x_110] = temp;
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
