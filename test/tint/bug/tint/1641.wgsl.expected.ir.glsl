SKIP: FAILED

#version 310 es

struct Normals {
  vec3 f;
};

vec4 main() {
  int zero = 0;
  return vec4(Normals[1](Normals(vec3(0.0f, 0.0f, 1.0f)))[zero].f, 1.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'float' :  entry point cannot return a value
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
