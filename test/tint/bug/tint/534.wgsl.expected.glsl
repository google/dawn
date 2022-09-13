#version 310 es

layout(binding = 2, std430) buffer OutputBuf_ssbo {
  uint result[];
} tint_symbol;

layout(binding = 3, std140) uniform Uniforms_ubo {
  uint dstTextureFlipY;
  uint isFloat16;
  uint isRGB10A2Unorm;
  uint channelCount;
} uniforms;

uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}

uniform highp sampler2D src_1;
uniform highp sampler2D dst_1;
void tint_symbol_1(uvec3 GlobalInvocationID) {
  ivec2 size = textureSize(src_1, 0);
  ivec2 dstTexCoord = ivec2(GlobalInvocationID.xy);
  ivec2 srcTexCoord = dstTexCoord;
  if ((uniforms.dstTextureFlipY == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1);
  }
  vec4 srcColor = texelFetch(src_1, srcTexCoord, 0);
  vec4 dstColor = texelFetch(dst_1, dstTexCoord, 0);
  bool success = true;
  uvec4 srcColorBits = uvec4(0u, 0u, 0u, 0u);
  uvec4 dstColorBits = uvec4(dstColor);
  {
    for(uint i = 0u; (i < uniforms.channelCount); i = (i + 1u)) {
      uint tint_symbol_2 = ConvertToFp16FloatValue(srcColor[i]);
      srcColorBits[i] = tint_symbol_2;
      bool tint_tmp = success;
      if (tint_tmp) {
        tint_tmp = (srcColorBits[i] == dstColorBits[i]);
      }
      success = (tint_tmp);
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(size.x)) + GlobalInvocationID.x);
  if (success) {
    tint_symbol.result[outputIndex] = 1u;
  } else {
    tint_symbol.result[outputIndex] = 0u;
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1(gl_GlobalInvocationID);
  return;
}
