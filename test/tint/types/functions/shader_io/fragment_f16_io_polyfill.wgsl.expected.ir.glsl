SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct Outputs {
  float16_t a;
  f16vec4 b;
};
precision highp float;
precision highp int;


Outputs main(float16_t loc1, f16vec4 loc2) {
  return Outputs((loc1 * 2.0hf), (loc2 * 3.0hf));
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'main' : function cannot take any parameter(s) 
ERROR: 0:12: 'structure' :  entry point cannot return a value
ERROR: 0:12: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
