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
      if ((GLF_live15i < 4)) {
      } else {
        break;
      }
      if ((GLF_live15i >= 3)) {
        break;
      }
      if ((GLF_live15c.y >= 1.0f)) {
        int x_10 = GLF_live15i;
        GLF_live15c[x_10] = 1.0f;
      }
      {
        GLF_live15i = (GLF_live15i + 1);
      }
      continue;
    }
  }
  GLF_live15d = vec4(0.0f);
  GLF_live15i_1 = 0;
  {
    while(true) {
      if ((GLF_live15i_1 < 4)) {
      } else {
        break;
      }
      if ((GLF_live15i_1 >= 3)) {
        break;
      }
      if ((GLF_live15d.y >= 1.0f)) {
        int x_15 = GLF_live15i_1;
        GLF_live15d[x_15] = 1.0f;
      }
      {
        GLF_live15i_1 = (GLF_live15i_1 + 1);
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
