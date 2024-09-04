SKIP: FAILED

#version 310 es

struct buf0 {
  float three;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
float func_() {
  float b = 0.0f;
  float x_34 = 0.0f;
  float x_34_phi = 0.0f;
  float x_48_phi = 0.0f;
  b = 2.0f;
  x_34_phi = 2.0f;
  {
    while(true) {
      x_34 = x_34_phi;
      float x_39 = x_7.three;
      if ((x_39 == 0.0f)) {
        x_48_phi = x_34;
        break;
      }
      float x_44 = x_7.three;
      if ((x_44 == 0.0f)) {
        return 1.0f;
      }
      b = 1.0f;
      {
        x_34_phi = 1.0f;
        x_48_phi = 1.0f;
        if (true) { break; }
      }
      continue;
    }
  }
  float x_48 = x_48_phi;
  return x_48;
}
void main_1() {
  float x_27 = func_();
  if ((x_27 == 1.0f)) {
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
