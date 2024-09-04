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
uniform buf1 x_6;
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float data[10] = float[10](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int x_41_phi = 0;
  int x_53_phi = 0;
  x_41_phi = 0;
  {
    while(true) {
      int x_42 = 0;
      int x_41 = x_41_phi;
      if ((x_41 < 10)) {
      } else {
        break;
      }
      {
        float x_49 = x_9.injectionSwitch.y;
        data[x_41] = (float((10 - x_41)) * x_49);
        x_42 = (x_41 + 1);
        x_41_phi = x_42;
      }
      continue;
    }
  }
  x_53_phi = 0;
  {
    while(true) {
      int x_54 = 0;
      int x_60_phi = 0;
      int x_53 = x_53_phi;
      if ((x_53 < 9)) {
      } else {
        break;
      }
      x_60_phi = 0;
      {
        while(true) {
          bool x_83 = false;
          bool x_84 = false;
          int x_61 = 0;
          bool x_85_phi = false;
          int x_60 = x_60_phi;
          if ((x_60 < 10)) {
          } else {
            break;
          }
          if ((x_60 < (x_53 + 1))) {
            {
              x_61 = (x_60 + 1);
              x_60_phi = x_61;
            }
            continue;
          }
          int x_70_save = x_53;
          float x_71 = data[x_70_save];
          int x_72_save = x_60;
          float x_73 = data[x_72_save];
          float x_75 = tint_symbol.y;
          float x_77 = x_6.resolution.y;
          if ((x_75 < (x_77 * 0.5f))) {
            x_83 = (x_71 > x_73);
            x_85_phi = x_83;
          } else {
            x_84 = (x_71 < x_73);
            x_85_phi = x_84;
          }
          bool x_85 = x_85_phi;
          if (x_85) {
            float x_88 = data[x_70_save];
            float x_89 = data[x_72_save];
            data[x_70_save] = x_89;
            data[x_72_save] = x_88;
          }
          {
            x_61 = (x_60 + 1);
            x_60_phi = x_61;
          }
          continue;
        }
      }
      {
        x_54 = (x_53 + 1);
        x_53_phi = x_54;
      }
      continue;
    }
  }
  float x_91 = tint_symbol.x;
  float x_93 = x_6.resolution.x;
  if ((x_91 < (x_93 * 0.5f))) {
    float x_100 = data[0];
    float x_103 = data[5];
    float x_106 = data[9];
    x_GLF_color = vec4((x_100 * 0.10000000149011611938f), (x_103 * 0.10000000149011611938f), (x_106 * 0.10000000149011611938f), 1.0f);
  } else {
    float x_110 = data[5];
    float x_113 = data[9];
    float x_116 = data[0];
    x_GLF_color = vec4((x_110 * 0.10000000149011611938f), (x_113 * 0.10000000149011611938f), (x_116 * 0.10000000149011611938f), 1.0f);
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
