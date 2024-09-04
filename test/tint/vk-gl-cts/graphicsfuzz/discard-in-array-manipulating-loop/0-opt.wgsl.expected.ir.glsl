SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  float data[10] = float[10](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  data = float[10](0.10000000149011611938f, 0.20000000298023223877f, 0.30000001192092895508f, 0.40000000596046447754f, 0.5f, 0.60000002384185791016f, 0.69999998807907104492f, 0.80000001192092895508f, 0.89999997615814208984f, 1.0f);
  i = 0;
  {
    while(true) {
      int x_7 = i;
      if ((x_7 < 10)) {
      } else {
        break;
      }
      float x_50 = tint_symbol.x;
      if ((x_50 < 0.0f)) {
        continue_execution = false;
      }
      int x_8 = i;
      float x_55 = data[x_8];
      data[0] = x_55;
      {
        int x_9 = i;
        i = (x_9 + 1);
      }
      continue;
    }
  }
  float x_58 = data[0];
  x_GLF_color = vec4(x_58, 0.0f, 0.0f, 1.0f);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
