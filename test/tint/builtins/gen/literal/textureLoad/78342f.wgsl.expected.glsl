SKIP: FAILED

Error parsing GLSL shader:
ERROR: 0:9: 'image load-store format' : not supported with this profile: es
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



Error parsing GLSL shader:
ERROR: 0:7: 'image load-store format' : not supported with this profile: es
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



Error parsing GLSL shader:
ERROR: 0:9: 'image load-store format' : not supported with this profile: es
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
layout(binding = 1, rg16_snorm) uniform highp readonly image2DArray f_arg_0;
vec4 textureLoad_78342f() {
  uint v_1 = (uint(imageSize(f_arg_0).z) - 1u);
  uint v_2 = min(uint(1), v_1);
  uvec2 v_3 = (uvec2(imageSize(f_arg_0).xy) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(ivec2(1)), v_3));
  vec4 res = imageLoad(f_arg_0, ivec3(v_4, int(v_2)));
  return res;
}
void main() {
  v.inner = textureLoad_78342f();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 1, rg16_snorm) uniform highp readonly image2DArray arg_0;
vec4 textureLoad_78342f() {
  uint v_1 = (uint(imageSize(arg_0).z) - 1u);
  uint v_2 = min(uint(1), v_1);
  uvec2 v_3 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(ivec2(1)), v_3));
  vec4 res = imageLoad(arg_0, ivec3(v_4, int(v_2)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_78342f();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, rg16_snorm) uniform highp readonly image2DArray v_arg_0;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec4 textureLoad_78342f() {
  uint v = (uint(imageSize(v_arg_0).z) - 1u);
  uint v_1 = min(uint(1), v);
  uvec2 v_2 = (uvec2(imageSize(v_arg_0).xy) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(ivec2(1)), v_2));
  vec4 res = imageLoad(v_arg_0, ivec3(v_3, int(v_1)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_4 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_4.pos = vec4(0.0f);
  v_4.prevent_dce = textureLoad_78342f();
  return v_4;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = vec4(v_5.pos.x, -(v_5.pos.y), ((2.0f * v_5.pos.z) - v_5.pos.w), v_5.pos.w);
  tint_interstage_location0 = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}

tint executable returned error: exit status 1
