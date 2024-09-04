SKIP: FAILED

#version 310 es

struct main_out {
  vec4 color_1;
};
precision highp float;
precision highp int;


vec4 color = vec4(0.0f);
vec3 drawShape_vf2_(inout vec2 pos) {
  bool c3 = false;
  bool x_35_phi = false;
  float x_32 = pos.y;
  bool x_33 = (x_32 < 1.0f);
  c3 = x_33;
  x_35_phi = x_33;
  {
    while(true) {
      bool x_35 = x_35_phi;
      if (x_35) {
      } else {
        break;
      }
      return vec3(1.0f);
    }
  }
  return vec3(1.0f);
}
void main_1() {
  vec2 param = vec2(0.0f);
  param = vec2(1.0f);
  vec3 x_29 = drawShape_vf2_(param);
  color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
main_out main() {
  main_1();
  return main_out(color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
