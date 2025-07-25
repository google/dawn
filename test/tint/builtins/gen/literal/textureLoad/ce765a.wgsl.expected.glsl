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
  ivec4 inner;
} v;
layout(binding = 1, rg8i) uniform highp readonly iimage3D f_arg_0;
ivec4 textureLoad_ce765a() {
  uvec3 v_1 = (uvec3(imageSize(f_arg_0)) - uvec3(1u));
  ivec4 res = imageLoad(f_arg_0, ivec3(min(uvec3(ivec3(1)), v_1)));
  return res;
}
void main() {
  v.inner = textureLoad_ce765a();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
layout(binding = 1, rg8i) uniform highp readonly iimage3D arg_0;
ivec4 textureLoad_ce765a() {
  uvec3 v_1 = (uvec3(imageSize(arg_0)) - uvec3(1u));
  ivec4 res = imageLoad(arg_0, ivec3(min(uvec3(ivec3(1)), v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_ce765a();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

layout(binding = 0, rg8i) uniform highp readonly iimage3D v_arg_0;
layout(location = 0) flat out ivec4 tint_interstage_location0;
ivec4 textureLoad_ce765a() {
  uvec3 v = (uvec3(imageSize(v_arg_0)) - uvec3(1u));
  ivec4 res = imageLoad(v_arg_0, ivec3(min(uvec3(ivec3(1)), v)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f), ivec4(0));
  v_1.pos = vec4(0.0f);
  v_1.prevent_dce = textureLoad_ce765a();
  return v_1;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = vec4(v_2.pos.x, -(v_2.pos.y), ((2.0f * v_2.pos.z) - v_2.pos.w), v_2.pos.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}

tint executable returned error: exit status 1
