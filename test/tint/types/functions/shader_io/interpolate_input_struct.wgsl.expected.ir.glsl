SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct In {
  float none;
  float tint_symbol;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
  float perspective_default;
  float linear_default;
};

void main(In tint_symbol_2) {
}
error: Error parsing GLSL shader:
ERROR: 0:19: 'main' : function cannot take any parameter(s) 
ERROR: 0:19: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
