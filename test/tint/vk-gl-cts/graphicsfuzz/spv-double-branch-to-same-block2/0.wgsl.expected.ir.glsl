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
  int x_40_phi = 0;
  int x_52_phi = 0;
  x_40_phi = 0;
  {
    while(true) {
      int x_41 = 0;
      int x_40 = x_40_phi;
      if ((x_40 < 10)) {
      } else {
        break;
      }
      {
        float x_48 = x_9.injectionSwitch.y;
        data[x_40] = (float((10 - x_40)) * x_48);
        x_41 = (x_40 + 1);
        x_40_phi = x_41;
      }
      continue;
    }
  }
  x_52_phi = 0;
  {
    while(true) {
      int x_53 = 0;
      int x_59_phi = 0;
      int x_52 = x_52_phi;
      if ((x_52 < 9)) {
      } else {
        break;
      }
      x_59_phi = 0;
      {
        while(true) {
          bool x_82 = false;
          bool x_83 = false;
          int x_60 = 0;
          bool x_84_phi = false;
          int x_59 = x_59_phi;
          if ((x_59 < 10)) {
          } else {
            break;
          }
          if ((x_59 < (x_52 + 1))) {
            {
              x_60 = (x_59 + 1);
              x_59_phi = x_60;
            }
            continue;
          }
          int x_69_save = x_52;
          float x_70 = data[x_69_save];
          int x_71_save = x_59;
          float x_72 = data[x_71_save];
          float x_74 = tint_symbol.y;
          float x_76 = x_6.resolution.y;
          if ((x_74 < (x_76 * 0.5f))) {
            x_82 = (x_70 > x_72);
            x_84_phi = x_82;
          } else {
            x_83 = (x_70 < x_72);
            x_84_phi = x_83;
          }
          bool x_84 = x_84_phi;
          if (x_84) {
            float x_87 = data[x_69_save];
            float x_88 = data[x_71_save];
            data[x_69_save] = x_88;
            data[x_71_save] = x_87;
          }
          {
            x_60 = (x_59 + 1);
            x_59_phi = x_60;
          }
          continue;
        }
      }
      {
        x_53 = (x_52 + 1);
        x_52_phi = x_53;
      }
      continue;
    }
  }
  float x_90 = tint_symbol.x;
  float x_92 = x_6.resolution.x;
  if ((x_90 < (x_92 * 0.5f))) {
    float x_99 = data[0];
    float x_102 = data[5];
    float x_105 = data[9];
    x_GLF_color = vec4((x_99 * 0.10000000149011611938f), (x_102 * 0.10000000149011611938f), (x_105 * 0.10000000149011611938f), 1.0f);
  } else {
    float x_109 = data[5];
    float x_112 = data[9];
    float x_115 = data[0];
    x_GLF_color = vec4((x_109 * 0.10000000149011611938f), (x_112 * 0.10000000149011611938f), (x_115 * 0.10000000149011611938f), 1.0f);
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
