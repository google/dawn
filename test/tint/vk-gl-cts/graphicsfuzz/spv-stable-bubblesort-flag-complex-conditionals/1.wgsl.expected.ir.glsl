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
  float x_179 = 0.0f;
  float x_178 = 0.0f;
  float x_185 = 0.0f;
  float x_184 = 0.0f;
  float x_160_phi = 0.0f;
  float x_180_phi = 0.0f;
  float x_186_phi = 0.0f;
  float x_149 = tint_symbol.y;
  float x_151 = x_9.resolution.y;
  bool x_153 = (x_149 < (x_151 / 2.0f));
  if (x_153) {
    x_158 = a;
    x_160_phi = x_158;
  } else {
    x_159 = 0.0f;
    x_160_phi = x_159;
  }
  float x_166 = 0.0f;
  float x_167 = 0.0f;
  float x_168_phi = 0.0f;
  float x_160 = x_160_phi;
  bool guard155 = true;
  if (false) {
  } else {
    if (guard155) {
      if (x_153) {
        x_166 = b;
        x_168_phi = x_166;
      } else {
        x_167 = 0.0f;
        x_168_phi = x_167;
      }
      float x_168 = x_168_phi;
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
    x_180_phi = x_179;
  } else {
    x_178 = a;
    x_180_phi = x_178;
  }
  float x_180 = x_180_phi;
  if (x_153) {
    x_185 = 0.0f;
    x_186_phi = x_185;
  } else {
    x_184 = b;
    x_186_phi = x_184;
  }
  float x_186 = x_186_phi;
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
      int x_59 = i;
      if ((x_59 < 10)) {
      } else {
        break;
      }
      int x_62 = i;
      int x_63 = i;
      float x_67 = x_13.injectionSwitch.y;
      data[x_62] = (float((10 - x_63)) * x_67);
      {
        int x_70 = i;
        i = (x_70 + 1);
      }
      continue;
    }
  }
  i_1 = 0;
  {
    while(true) {
      int x_76 = i_1;
      if ((x_76 < 9)) {
      } else {
        break;
      }
      j = 0;
      {
        while(true) {
          int x_83 = j;
          if ((x_83 < 10)) {
          } else {
            break;
          }
          int x_86 = j;
          int x_87 = i_1;
          if ((x_86 < (x_87 + 1))) {
            {
              int x_113 = j;
              j = (x_113 + 1);
            }
            continue;
          }
          int x_92 = i_1;
          int x_93 = j;
          float x_95 = data[x_92];
          param = x_95;
          float x_97 = data[x_93];
          param_1 = x_97;
          bool x_98 = checkSwap_f1_f1_(param, param_1);
          doSwap = x_98;
          bool x_99 = doSwap;
          if (x_99) {
            int x_102 = i_1;
            float x_104 = data[x_102];
            temp = x_104;
            int x_105 = i_1;
            int x_106 = j;
            float x_108 = data[x_106];
            data[x_105] = x_108;
            int x_110 = j;
            float x_111 = temp;
            data[x_110] = x_111;
          }
          {
            int x_113 = j;
            j = (x_113 + 1);
          }
          continue;
        }
      }
      {
        int x_115 = i_1;
        i_1 = (x_115 + 1);
      }
      continue;
    }
  }
  float x_118 = tint_symbol.x;
  float x_120 = x_9.resolution.x;
  if ((x_118 < (x_120 / 2.0f))) {
    float x_127 = data[0];
    float x_130 = data[5];
    float x_133 = data[9];
    x_GLF_color = vec4((x_127 / 10.0f), (x_130 / 10.0f), (x_133 / 10.0f), 1.0f);
  } else {
    float x_137 = data[5];
    float x_140 = data[9];
    float x_143 = data[0];
    x_GLF_color = vec4((x_137 / 10.0f), (x_140 / 10.0f), (x_143 / 10.0f), 1.0f);
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
