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
  if ((x_6.injectionSwitch.y < 0.0f)) {
    {
      while(true) {
        if (true) {
        } else {
          break;
        }
        GLF_dead6currentNode = donor_replacementGLF_dead6tree[GLF_dead6index];
        GLF_dead6index = GLF_dead6currentNode;
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
