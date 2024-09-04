SKIP: FAILED

#version 310 es

vec4 main() {
  vec3 light = vec3(1.20000004768371582031f, 1.0f, 2.0f);
  vec3 negative_light = -(light);
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'float' :  entry point cannot return a value
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
