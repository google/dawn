SKIP: FAILED

#version 310 es

struct DeclaredAfterUsage {
  float f;
};

uniform DeclaredAfterUsage declared_after_usage;
vec4 main() {
  return vec4(declared_after_usage.f);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' :  entry point cannot return a value
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
