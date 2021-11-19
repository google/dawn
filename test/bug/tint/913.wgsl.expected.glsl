#version 310 es
precision mediump float;


uniform highp sampler2D src;
uniform highp sampler2D dst;
layout (binding = 2) buffer OutputBuf_1 {
  uint result[];
} tint_symbol;
layout (binding = 3) uniform Uniforms_1 {
  uint dstTextureFlipY;
  uint channelCount;
  uvec2 srcCopyOrigin;
  uvec2 dstCopyOrigin;
  uvec2 copySize;
} uniforms;

bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.001f);
}

struct tint_symbol_3 {
  uvec3 GlobalInvocationID;
};

void tint_symbol_1_inner(uvec3 GlobalInvocationID) {
  ivec2 srcSize = textureSize(src, 0);
  ivec2 dstSize = textureSize(dst, 0);
  uvec2 dstTexCoord = uvec2(GlobalInvocationID.xy);
  vec4 nonCoveredColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  bool tint_tmp_2 = (dstTexCoord.x < uniforms.dstCopyOrigin.x);
  if (!tint_tmp_2) {
    tint_tmp_2 = (dstTexCoord.y < uniforms.dstCopyOrigin.y);
  }
  bool tint_tmp_1 = (tint_tmp_2);
  if (!tint_tmp_1) {
    tint_tmp_1 = (dstTexCoord.x >= (uniforms.dstCopyOrigin.x + uniforms.copySize.x));
  }
  bool tint_tmp = (tint_tmp_1);
  if (!tint_tmp) {
    tint_tmp = (dstTexCoord.y >= (uniforms.dstCopyOrigin.y + uniforms.copySize.y));
  }
  if ((tint_tmp)) {
    bool tint_tmp_3 = success;
    if (tint_tmp_3) {
      tint_tmp_3 = all(equal(texelFetch(dst, ivec2(dstTexCoord), 0), nonCoveredColor));
    }
    success = (tint_tmp_3);
  } else {
    uvec2 srcTexCoord = ((dstTexCoord - uniforms.dstCopyOrigin) + uniforms.srcCopyOrigin);
    if ((uniforms.dstTextureFlipY == 1u)) {
      srcTexCoord.y = ((uint(srcSize.y) - srcTexCoord.y) - 1u);
    }
    vec4 srcColor = texelFetch(src, ivec2(srcTexCoord), 0);
    vec4 dstColor = texelFetch(dst, ivec2(dstTexCoord), 0);
    if ((uniforms.channelCount == 2u)) {
      bool tint_tmp_5 = success;
      if (tint_tmp_5) {
        tint_tmp_5 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_tmp_4 = (tint_tmp_5);
      if (tint_tmp_4) {
        tint_tmp_4 = aboutEqual(dstColor.g, srcColor.g);
      }
      success = (tint_tmp_4);
    } else {
      bool tint_tmp_9 = success;
      if (tint_tmp_9) {
        tint_tmp_9 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_tmp_8 = (tint_tmp_9);
      if (tint_tmp_8) {
        tint_tmp_8 = aboutEqual(dstColor.g, srcColor.g);
      }
      bool tint_tmp_7 = (tint_tmp_8);
      if (tint_tmp_7) {
        tint_tmp_7 = aboutEqual(dstColor.b, srcColor.b);
      }
      bool tint_tmp_6 = (tint_tmp_7);
      if (tint_tmp_6) {
        tint_tmp_6 = aboutEqual(dstColor.a, srcColor.a);
      }
      success = (tint_tmp_6);
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(dstSize.x)) + GlobalInvocationID.x);
  if (success) {
    tint_symbol.result[outputIndex] = 1u;
  } else {
    tint_symbol.result[outputIndex] = 0u;
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


