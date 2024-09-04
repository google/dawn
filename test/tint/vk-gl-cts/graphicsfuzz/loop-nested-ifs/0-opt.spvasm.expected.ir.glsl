SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int j = 0;
  float a = 0.0f;
  j = 0;
  {
    while(true) {
      if ((j < 2)) {
      } else {
        break;
      }
      if ((j < 1)) {
        x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      }
      if ((j != 3)) {
        if ((j != 4)) {
          if ((j == 5)) {
            x_GLF_color[0u] = 4.0f;
          } else {
            a = 4.0f;
            x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
          }
        }
      }
      {
        j = (j + 1);
      }
      continue;
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
