SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int GLF_live9r = 0;
  float g = 0.0f;
  {
    while(true) {
      if (true) {
      } else {
        break;
      }
      if (true) {
        break;
      }
      int x_31 = GLF_live9r;
      int x_32 = min(max(x_31, 0), 1);
      {
      }
      continue;
    }
  }
  g = 3.0f;
  float x_33 = g;
  x_GLF_color = vec4(smoothstep(0.0f, 1.0f, x_33), 0.0f, 0.0f, 1.0f);
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
