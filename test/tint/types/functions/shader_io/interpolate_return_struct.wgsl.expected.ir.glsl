SKIP: FAILED

#version 310 es

struct Out {
  vec4 pos;
  float none;
  float tint_symbol;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
};

Out main() {
  return Out(vec4(0.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'structure' :  entry point cannot return a value
ERROR: 0:15: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
