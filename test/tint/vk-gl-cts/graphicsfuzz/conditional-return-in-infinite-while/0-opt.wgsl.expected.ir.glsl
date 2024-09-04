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


int GLF_live6tree[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
int GLF_live6search_() {
  int GLF_live6index = 0;
  int GLF_live6currentNode = 0;
  GLF_live6index = 0;
  {
    while(true) {
      if (true) {
      } else {
        break;
      }
      int x_10 = GLF_live6index;
      int x_11 = GLF_live6tree[x_10];
      GLF_live6currentNode = x_11;
      int x_12 = GLF_live6currentNode;
      if ((x_12 != 1)) {
        return 1;
      }
      GLF_live6index = 1;
      {
      }
      continue;
    }
  }
  return 1;
}
void main_1() {
  float x_40 = x_9.injectionSwitch.x;
  if ((x_40 > 1.0f)) {
    int x_13 = GLF_live6search_();
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
