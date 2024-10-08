SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleCompareLevel_b6e47c() {
  float res = textureOffset(arg_0_arg_1, vec4(vec2(1.0f), float(1), 1.0f), ivec2(1));
  return res;
}
void main() {
  v.tint_symbol = textureSampleCompareLevel_b6e47c();
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'sampler' : TextureOffset does not support sampler2DArrayShadow :  ES Profile
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleCompareLevel_b6e47c() {
  float res = textureOffset(arg_0_arg_1, vec4(vec2(1.0f), float(1), 1.0f), ivec2(1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureSampleCompareLevel_b6e47c();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'sampler' : TextureOffset does not support sampler2DArrayShadow :  ES Profile
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

uniform highp sampler2DArrayShadow arg_0_arg_1;
layout(location = 0) flat out float vertex_main_loc0_Output;
float textureSampleCompareLevel_b6e47c() {
  float res = textureOffset(arg_0_arg_1, vec4(vec2(1.0f), float(1), 1.0f), ivec2(1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0f);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureSampleCompareLevel_b6e47c();
  return tint_symbol;
}
void main() {
  VertexOutput v = vertex_main_inner();
  gl_Position = v.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'sampler' : TextureOffset does not support sampler2DArrayShadow :  ES Profile
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
