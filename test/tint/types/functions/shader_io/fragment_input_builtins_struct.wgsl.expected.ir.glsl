SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct FragmentInputs {
  vec4 position;
  bool front_facing;
  uint sample_index;
  uint sample_mask;
};

void main(FragmentInputs inputs) {
  if (inputs.front_facing) {
    vec4 foo = inputs.position;
    uint bar = (inputs.sample_index + inputs.sample_mask);
  }
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'main' : function cannot take any parameter(s) 
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
