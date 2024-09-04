SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t tint_symbol[];
float16_t tint_symbol_1[];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1[0] = tint_symbol[0];
}
error: Error parsing GLSL shader:
ERROR: 0:4: '' : array size required 
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1
