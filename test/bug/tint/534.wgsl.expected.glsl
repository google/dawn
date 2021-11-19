#version 310 es
precision mediump float;


uniform highp sampler2D src;
uniform highp sampler2D dst;
layout (binding = 2) buffer OutputBuf_1 {
  uint result[];
} tint_symbol;
layout (binding = 3) uniform Uniforms_1 {
  uint dstTextureFlipY;
  uint isFloat16;
  uint isRGB10A2Unorm;
  uint channelCount;
} uniforms;

uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}

struct tint_symbol_3 {
  uvec3 GlobalInvocationID;
};

void tint_symbol_1_inner(uvec3 GlobalInvocationID) {
  ivec2 size = textureSize(src, 0);
  ivec2 dstTexCoord = ivec2(GlobalInvocationID.xy);
  ivec2 srcTexCoord = dstTexCoord;
  if ((uniforms.dstTextureFlipY == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1);
  }
  vec4 srcColor = texelFetch(src, srcTexCoord, 0);
  vec4 dstColor = texelFetch(dst, dstTexCoord, 0);
  bool success = true;
  uvec4 srcColorBits = uvec4(0u, 0u, 0u, 0u);
  uvec4 dstColorBits = uvec4(dstColor);
  {
    for(uint i = 0u; (i < uniforms.channelCount); i = (i + 1u)) {
      srcColorBits[i] = ConvertToFp16FloatValue(srcColor[i]);
      bool tint_tmp = success;
      if (tint_tmp) {
        tint_tmp = (srcColorBits[i] == dstColorBits[i]);
      }
      success = (tint_tmp);
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(size.x)) + GlobalInvocationID.x);
  if (success) {
    tint_symbol.result[outputIndex] = uint(1);
  } else {
    tint_symbol.result[outputIndex] = uint(0);
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol_1(tint_symbol_3 tint_symbol_2) {
  tint_symbol_1_inner(tint_symbol_2.GlobalInvocationID);
  return;
}
void main() {
  tint_symbol_3 inputs;
  inputs.GlobalInvocationID = gl_GlobalInvocationID;
  tint_symbol_1(inputs);
}


