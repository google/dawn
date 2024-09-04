SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct FragmentOutputs {
  int loc0;
  float frag_depth;
  uint loc1;
  float loc2;
  uint sample_mask;
  vec4 loc3;
  float16_t loc4;
  f16vec3 loc5;
};
precision highp float;
precision highp int;


FragmentOutputs main() {
  return FragmentOutputs(1, 2.0f, 1u, 1.0f, 2u, vec4(1.0f, 2.0f, 3.0f, 4.0f), 2.25hf, f16vec3(3.0hf, 5.0hf, 8.0hf));
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
