SKIP: FAILED

#version 310 es

vec4 main() {
  int v1 = 1;
  uint v2 = 1u;
  float v3 = 1.0f;
  ivec3 v4 = ivec3(1);
  uvec3 v5 = uvec3(1u);
  vec3 v6 = vec3(1.0f);
  mat3 v7 = mat3(vec3(1.0f), vec3(1.0f), vec3(1.0f));
  float v9[10] = float[10](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
