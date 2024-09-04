SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
void main_1() {
  int GLF_dead6index = 0;
  int GLF_dead6currentNode = 0;
  int donor_replacementGLF_dead6tree[1] = int[1](0);
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  GLF_dead6index = 0;
  float x_34 = x_6.injectionSwitch.y;
  if ((x_34 < 0.0f)) {
    {
      while(true) {
        if (true) {
        } else {
          break;
        }
        int x_9 = GLF_dead6index;
        int x_10 = donor_replacementGLF_dead6tree[x_9];
        GLF_dead6currentNode = x_10;
        int x_11 = GLF_dead6currentNode;
        GLF_dead6index = x_11;
        {
        }
        continue;
      }
    }
  }
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
