SKIP: FAILED

#version 310 es

struct buf0 {
  int zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
int func_i1_(inout int x) {
  int x_45 = x;
  if ((x_45 == 10)) {
    continue_execution = false;
  }
  int x_49 = x;
  return x_49;
}
void main_1() {
  int a = 0;
  int b = 0;
  int param = 0;
  int x_37 = 0;
  int x_35_phi = 0;
  a = 0;
  int x_33 = x_9.zero;
  b = x_33;
  x_35_phi = x_33;
  {
    while(true) {
      int x_35 = x_35_phi;
      param = x_35;
      x_37 = func_i1_(param);
      a = x_37;
      int x_36 = (x_35 + 1);
      b = x_36;
      x_35_phi = x_36;
      if ((x_36 < 4)) {
      } else {
        break;
      }
      {
      }
      continue;
    }
  }
  if ((x_37 == 3)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
  }
}
main_out main() {
  main_1();
  main_out v = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v;
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
