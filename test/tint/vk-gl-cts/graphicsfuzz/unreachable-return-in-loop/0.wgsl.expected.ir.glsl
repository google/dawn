SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  bool x_21_phi = false;
  x_21_phi = false;
  {
    while(true) {
      bool x_25 = false;
      bool x_25_phi = false;
      bool x_30_phi = false;
      bool x_21 = x_21_phi;
      x_25_phi = x_21;
      {
        while(true) {
          x_25 = x_25_phi;
          x_30_phi = x_25;
          if (false) {
          } else {
            break;
          }
          x_30_phi = true;
          break;
        }
      }
      bool x_30 = x_30_phi;
      if (x_30) {
        break;
      }
      x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      break;
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
