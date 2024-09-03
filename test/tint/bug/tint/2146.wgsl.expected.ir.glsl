#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uvec3 localId = uvec3(0u);
uint localIndex = 0u;
uvec3 globalId = uvec3(0u);
uvec3 numWorkgroups = uvec3(0u);
uvec3 workgroupId = uvec3(0u);
uint globalId2Index() {
  return globalId.x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16vec4 a = f16vec4(0.0hf);
  float16_t b = 1.0hf;
  a[0] = (a.x + b);
}
