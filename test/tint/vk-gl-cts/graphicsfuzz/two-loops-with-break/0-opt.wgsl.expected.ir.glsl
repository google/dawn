SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 GLF_live15c = vec4(0.0f);
  int GLF_live15i = 0;
  vec4 GLF_live15d = vec4(0.0f);
  int GLF_live15i_1 = 0;
  GLF_live15c = vec4(0.0f);
  GLF_live15i = 0;
  {
    while(true) {
      int x_8 = GLF_live15i;
      if ((x_8 < 4)) {
      } else {
        break;
      }
      int x_9 = GLF_live15i;
      if ((x_9 >= 3)) {
        break;
      }
      float x_49 = GLF_live15c.y;
      if ((x_49 >= 1.0f)) {
        int x_10 = GLF_live15i;
        GLF_live15c[x_10] = 1.0f;
      }
      {
        int x_11 = GLF_live15i;
        GLF_live15i = (x_11 + 1);
      }
      continue;
    }
  }
  GLF_live15d = vec4(0.0f);
  GLF_live15i_1 = 0;
  {
    while(true) {
      int x_13 = GLF_live15i_1;
      if ((x_13 < 4)) {
      } else {
        break;
      }
      int x_14 = GLF_live15i_1;
      if ((x_14 >= 3)) {
        break;
      }
      float x_64 = GLF_live15d.y;
      if ((x_64 >= 1.0f)) {
        int x_15 = GLF_live15i_1;
        GLF_live15d[x_15] = 1.0f;
      }
      {
        int x_16 = GLF_live15i_1;
        GLF_live15i_1 = (x_16 + 1);
      }
      continue;
    }
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
