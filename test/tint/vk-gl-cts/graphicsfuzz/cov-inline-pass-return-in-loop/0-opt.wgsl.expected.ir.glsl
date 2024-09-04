SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
float func_() {
  int i = 0;
  i = 0;
  {
    while(true) {
      int x_35 = i;
      if ((x_35 < 10)) {
      } else {
        break;
      }
      int x_38 = i;
      if ((x_38 > 5)) {
        int x_42 = i;
        i = (x_42 + 1);
      }
      int x_44 = i;
      if ((x_44 > 8)) {
        return 0.0f;
      }
      {
        int x_48 = i;
        i = (x_48 + 1);
      }
      continue;
    }
  }
  return 1.0f;
}
void main_1() {
  if (false) {
    float x_28 = func_();
    x_GLF_color = vec4(x_28, x_28, x_28, x_28);
  } else {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
