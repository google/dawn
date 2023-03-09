#version 310 es

uvec4 tint_select(uvec4 param_0, uvec4 param_1, bvec4 param_2) {
    return uvec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


uvec4 tint_ftou(vec4 v) {
  return tint_select(uvec4(4294967295u), tint_select(uvec4(v), uvec4(0u), lessThan(v, vec4(0.0f))), lessThan(v, vec4(4294967040.0f)));
}

struct Uniforms {
  uint dstTextureFlipY;
  uint isFloat16;
  uint isRGB10A2Unorm;
  uint channelCount;
};

layout(binding = 2, std430) buffer OutputBuf_ssbo {
  uint result[];
} tint_symbol;

layout(binding = 3, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}

uniform highp sampler2D src_1;
uniform highp sampler2D dst_1;
void tint_symbol_1(uvec3 GlobalInvocationID) {
  uvec2 size = uvec2(textureSize(src_1, 0));
  uvec2 dstTexCoord = GlobalInvocationID.xy;
  uvec2 srcTexCoord = dstTexCoord;
  if ((uniforms.inner.dstTextureFlipY == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1u);
  }
  vec4 srcColor = texelFetch(src_1, ivec2(srcTexCoord), 0);
  vec4 dstColor = texelFetch(dst_1, ivec2(dstTexCoord), 0);
  bool success = true;
  uvec4 srcColorBits = uvec4(0u, 0u, 0u, 0u);
  uvec4 dstColorBits = tint_ftou(dstColor);
  {
    for(uint i = 0u; (i < uniforms.inner.channelCount); i = (i + 1u)) {
      uint tint_symbol_2 = i;
      srcColorBits[tint_symbol_2] = ConvertToFp16FloatValue(srcColor[i]);
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
