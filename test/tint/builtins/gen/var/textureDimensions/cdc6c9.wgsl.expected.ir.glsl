SKIP: FAILED

#version 310 es

struct tint_GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 visibleSize;
  vec2 plane1CoordFactor;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

uvec2 prevent_dce;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
uniform tint_ExternalTextureParams arg_0_params;
uvec2 textureDimensions_cdc6c9() {
  uvec2 res = (arg_0_params.visibleSize + uvec2(1u));
  return res;
}
void main() {
  prevent_dce = textureDimensions_cdc6c9();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = textureDimensions_cdc6c9();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec2(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureDimensions_cdc6c9();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct tint_GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 visibleSize;
  vec2 plane1CoordFactor;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

uvec2 prevent_dce;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
uniform tint_ExternalTextureParams arg_0_params;
uvec2 textureDimensions_cdc6c9() {
  uvec2 res = (arg_0_params.visibleSize + uvec2(1u));
  return res;
}
void main() {
  prevent_dce = textureDimensions_cdc6c9();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = textureDimensions_cdc6c9();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec2(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureDimensions_cdc6c9();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:51: 'main' : function already has a body 
ERROR: 0:51: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct tint_GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 visibleSize;
  vec2 plane1CoordFactor;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

uvec2 prevent_dce;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
uniform tint_ExternalTextureParams arg_0_params;
uvec2 textureDimensions_cdc6c9() {
  uvec2 res = (arg_0_params.visibleSize + uvec2(1u));
  return res;
}
void main() {
  prevent_dce = textureDimensions_cdc6c9();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = textureDimensions_cdc6c9();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec2(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureDimensions_cdc6c9();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:50: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:50: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
