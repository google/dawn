SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  bool b = false;
  int i = 0;
  float a = 0.0f;
  b = false;
  i = 1;
  {
    while(true) {
      int x_7 = i;
      if ((x_7 > 0)) {
      } else {
        break;
      }
      int x_8 = i;
      a = (3.0f - float(x_8));
      float x_40 = a;
      if (((2.0f - x_40) == 0.0f)) {
        b = true;
      }
      {
        int x_9 = i;
        i = (x_9 - 1);
      }
      continue;
    }
  }
  bool x_45 = b;
  if (x_45) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
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
