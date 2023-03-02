#version 310 es

struct Uniforms {
  uint dstTextureFlipY;
  uint channelCount;
  uvec2 srcCopyOrigin;
  uvec2 dstCopyOrigin;
  uvec2 copySize;
};

layout(binding = 2, std430) buffer OutputBuf_ssbo {
  uint result[];
} tint_symbol;

layout(binding = 3, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.00100000004749745131f);
}

uniform highp sampler2D src_1;
uniform highp sampler2D dst_1;
void tint_symbol_1(uvec3 GlobalInvocationID) {
  uvec2 srcSize = uvec2(textureSize(src_1, 0));
  uvec2 dstSize = uvec2(textureSize(dst_1, 0));
  uvec2 dstTexCoord = uvec2(GlobalInvocationID.xy);
  vec4 nonCoveredColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  bool tint_tmp_2 = (dstTexCoord.x < uniforms.inner.dstCopyOrigin.x);
  if (!tint_tmp_2) {
    tint_tmp_2 = (dstTexCoord.y < uniforms.inner.dstCopyOrigin.y);
  }
  bool tint_tmp_1 = (tint_tmp_2);
  if (!tint_tmp_1) {
    tint_tmp_1 = (dstTexCoord.x >= (uniforms.inner.dstCopyOrigin.x + uniforms.inner.copySize.x));
  }
  bool tint_tmp = (tint_tmp_1);
  if (!tint_tmp) {
    tint_tmp = (dstTexCoord.y >= (uniforms.inner.dstCopyOrigin.y + uniforms.inner.copySize.y));
  }
  if ((tint_tmp)) {
    bool tint_tmp_3 = success;
    if (tint_tmp_3) {
      tint_tmp_3 = all(equal(texelFetch(dst_1, ivec2(dstTexCoord), 0), nonCoveredColor));
    }
    success = (tint_tmp_3);
  } else {
    uvec2 srcTexCoord = ((dstTexCoord - uniforms.inner.dstCopyOrigin) + uniforms.inner.srcCopyOrigin);
    if ((uniforms.inner.dstTextureFlipY == 1u)) {
      srcTexCoord.y = ((srcSize.y - srcTexCoord.y) - 1u);
    }
    vec4 srcColor = texelFetch(src_1, ivec2(srcTexCoord), 0);
    vec4 dstColor = texelFetch(dst_1, ivec2(dstTexCoord), 0);
    if ((uniforms.inner.channelCount == 2u)) {
      bool tint_symbol_3 = success;
      if (tint_symbol_3) {
        tint_symbol_3 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_symbol_2 = tint_symbol_3;
      if (tint_symbol_2) {
        tint_symbol_2 = aboutEqual(dstColor.g, srcColor.g);
      }
      success = tint_symbol_2;
    } else {
      bool tint_symbol_7 = success;
      if (tint_symbol_7) {
        tint_symbol_7 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_symbol_6 = tint_symbol_7;
      if (tint_symbol_6) {
        tint_symbol_6 = aboutEqual(dstColor.g, srcColor.g);
      }
      bool tint_symbol_5 = tint_symbol_6;
      if (tint_symbol_5) {
        tint_symbol_5 = aboutEqual(dstColor.b, srcColor.b);
      }
      bool tint_symbol_4 = tint_symbol_5;
      if (tint_symbol_4) {
        tint_symbol_4 = aboutEqual(dstColor.a, srcColor.a);
      }
      success = tint_symbol_4;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * dstSize.x) + GlobalInvocationID.x);
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
