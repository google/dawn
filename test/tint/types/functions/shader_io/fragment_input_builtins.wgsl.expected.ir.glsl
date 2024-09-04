SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


void main(vec4 position, bool front_facing, uint sample_index, uint sample_mask) {
  if (front_facing) {
    vec4 foo = position;
    uint bar = (sample_index + sample_mask);
  }
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'main' : function cannot take any parameter(s) 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
