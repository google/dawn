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
  uvec2 inner;
} v;
layout(binding = 1, rg8i) uniform highp readonly iimage2D f_arg_0;
uvec2 textureDimensions_865495() {
  uvec2 res = uvec2(imageSize(f_arg_0));
  return res;
}
void main() {
  v.inner = textureDimensions_865495();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec2 inner;
} v;
layout(binding = 1, rg8i) uniform highp readonly iimage2D arg_0;
uvec2 textureDimensions_865495() {
  uvec2 res = uvec2(imageSize(arg_0));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_865495();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

layout(binding = 0, rg8i) uniform highp readonly iimage2D v_arg_0;
layout(location = 0) flat out uvec2 tint_interstage_location0;
uvec2 textureDimensions_865495() {
  uvec2 res = uvec2(imageSize(v_arg_0));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), uvec2(0u));
  v.pos = vec4(0.0f);
  v.prevent_dce = textureDimensions_865495();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}

tint executable returned error: exit status 1
