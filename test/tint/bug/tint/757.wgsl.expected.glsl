#version 310 es

struct Constants {
  int level;
};

layout(binding = 3, std430) buffer Result_ssbo {
  float values[];
} result;

uniform highp sampler2DArray myTexture_1;
void tint_symbol(uvec3 GlobalInvocationID) {
  uint flatIndex = (((4u * GlobalInvocationID.z) + (2u * GlobalInvocationID.y)) + GlobalInvocationID.x);
  flatIndex = (flatIndex * 1u);
  vec4 texel = texelFetch(myTexture_1, ivec3(ivec2(GlobalInvocationID.xy), 0), 0);
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      result.values[(flatIndex + i)] = texel.r;
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_GlobalInvocationID);
  return;
}
