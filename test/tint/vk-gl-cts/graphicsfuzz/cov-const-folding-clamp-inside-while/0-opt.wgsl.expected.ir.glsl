SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int i = 0;
  int j = 0;
  i = 0;
  j = 1;
  {
    while(true) {
      int x_28 = i;
      int x_29 = j;
      if ((x_28 < min(max(x_29, 5), 9))) {
      } else {
        break;
      }
      int x_33 = i;
      i = (x_33 + 1);
      int x_35 = j;
      j = (x_35 + 1);
      {
      }
      continue;
    }
  }
  int x_37 = i;
  int x_39 = j;
  if (((x_37 == 9) & (x_39 == 10))) {
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
