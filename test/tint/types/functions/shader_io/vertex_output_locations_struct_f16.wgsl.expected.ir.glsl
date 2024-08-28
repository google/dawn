SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct VertexOutputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  vec4 position;
  float16_t loc4;
  f16vec3 loc5;
};

VertexOutputs main() {
  return VertexOutputs(1, 1u, 1.0f, vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(0.0f), 2.25hf, f16vec3(3.0hf, 5.0hf, 8.0hf));
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'structure' :  entry point cannot return a value
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
