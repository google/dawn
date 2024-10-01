SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
layout(binding = 0, rgba8) uniform highp readonly image1D arg_0;
vec4 textureLoad_519ab5() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, int(arg_1));
  return res;
}
void main() {
  v.tint_symbol = textureLoad_519ab5();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'image1D' : Reserved word. 
WARNING: 0:9: 'layout' : useless application of layout qualifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
layout(binding = 0, rgba8) uniform highp readonly image1D arg_0;
vec4 textureLoad_519ab5() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, int(arg_1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureLoad_519ab5();
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'image1D' : Reserved word. 
WARNING: 0:7: 'layout' : useless application of layout qualifier 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, rgba8) uniform highp readonly image1D arg_0;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 textureLoad_519ab5() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, int(arg_1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_519ab5();
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
ERROR: 0:9: 'image1D' : Reserved word. 
WARNING: 0:9: 'layout' : useless application of layout qualifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
