SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct FragmentInputs0 {
  vec4 position;
  int loc0;
};

struct FragmentInputs1 {
  vec4 loc3;
  uint sample_mask;
};

void main(FragmentInputs0 inputs0, bool front_facing, uint loc1, uint sample_index, FragmentInputs1 inputs1, float loc2) {
  if (front_facing) {
    vec4 foo = inputs0.position;
    uint bar = (sample_index + inputs1.sample_mask);
    int i = inputs0.loc0;
    uint u = loc1;
    float f = loc2;
    vec4 v = inputs1.loc3;
  }
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'main' : function cannot take any parameter(s) 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
