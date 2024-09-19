SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec2 tint_symbol;
} v;
uniform highp sampler2D arg_0;
uvec2 textureDimensions_13f8db() {
  highp sampler2D v_1 = arg_0;
  uvec2 res = uvec2(textureSize(v_1, int(1u)));
  return res;
}
void main() {
  v.tint_symbol = textureDimensions_13f8db();
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'sampler2D' : sampler/image types can only be used in uniform variables or function parameters: v_1
ERROR: 0:11: '=' :  cannot convert from ' uniform highp sampler2D' to ' temp highp sampler2D'
ERROR: 0:11: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec2 tint_symbol;
} v;
uniform highp sampler2D arg_0;
uvec2 textureDimensions_13f8db() {
  highp sampler2D v_1 = arg_0;
  uvec2 res = uvec2(textureSize(v_1, int(1u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureDimensions_13f8db();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'sampler2D' : sampler/image types can only be used in uniform variables or function parameters: v_1
ERROR: 0:9: '=' :  cannot convert from ' uniform highp sampler2D' to ' temp highp sampler2D'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

uniform highp sampler2D arg_0;
layout(location = 0) flat out uvec2 vertex_main_loc0_Output;
uvec2 textureDimensions_13f8db() {
  highp sampler2D v = arg_0;
  uvec2 res = uvec2(textureSize(v, int(1u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec2(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureDimensions_13f8db();
  return tint_symbol;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'sampler2D' : sampler/image types can only be used in uniform variables or function parameters: v
ERROR: 0:12: '=' :  cannot convert from ' uniform highp sampler2D' to ' temp highp sampler2D'
ERROR: 0:12: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
