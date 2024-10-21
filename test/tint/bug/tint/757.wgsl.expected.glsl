#version 310 es

layout(binding = 3, std430)
buffer Result_1_ssbo {
  float values[];
} result;
uniform highp sampler2DArray myTexture;
void tint_symbol_inner(uvec3 GlobalInvocationID) {
  uint flatIndex = (((4u * GlobalInvocationID[2u]) + (2u * GlobalInvocationID[1u])) + GlobalInvocationID[0u]);
  flatIndex = (flatIndex * 1u);
  ivec2 v = ivec2(ivec2(GlobalInvocationID.xy));
  ivec3 v_1 = ivec3(v, int(0));
  vec4 texel = texelFetch(myTexture, v_1, int(0));
  {
    uint i = 0u;
    while(true) {
      if ((i < 1u)) {
      } else {
        break;
      }
      uint v_2 = (flatIndex + i);
      result.values[v_2] = texel.x;
      {
        i = (i + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_GlobalInvocationID);
}
