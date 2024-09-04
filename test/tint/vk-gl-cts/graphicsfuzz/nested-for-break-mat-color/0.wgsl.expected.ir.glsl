SKIP: FAILED

#version 310 es

struct buf0 {
  float one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  mat4 m44 = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  int x_10_phi = 0;
  m44 = mat4(vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(5.0f, 6.0f, 7.0f, 8.0f), vec4(9.0f, 10.0f, 11.0f, 12.0f), vec4(13.0f, 14.0f, 15.0f, 16.0f));
  x_10_phi = 0;
  {
    while(true) {
      int x_9 = 0;
      int x_11_phi = 0;
      int x_10 = x_10_phi;
      if ((x_10 < 4)) {
      } else {
        break;
      }
      float x_63 = tint_symbol.y;
      if ((x_63 < 0.0f)) {
        break;
      }
      x_11_phi = 0;
      {
        while(true) {
          int x_8 = 0;
          int x_11 = x_11_phi;
          if ((x_11 < 4)) {
          } else {
            break;
          }
          {
            float x_72 = x_7.one;
            float x_74 = m44[x_10][x_11];
            m44[x_10][x_11] = (x_74 + x_72);
            x_8 = (x_11 + 1);
            x_11_phi = x_8;
          }
          continue;
        }
      }
      {
        x_9 = (x_10 + 1);
        x_10_phi = x_9;
      }
      continue;
    }
  }
  float x_77 = m44[1].y;
  vec4 x_79_1 = vec4(0.0f);
  x_79_1[0u] = (x_77 - 6.0f);
  vec4 x_79 = x_79_1;
  float x_81 = m44[2].z;
  vec4 x_83_1 = x_79;
  x_83_1[3u] = (x_81 - 11.0f);
  vec4 x_83 = x_83_1;
  x_GLF_color = x_83;
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
