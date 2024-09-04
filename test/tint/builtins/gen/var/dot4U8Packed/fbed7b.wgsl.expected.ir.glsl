SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uint prevent_dce;
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = (uvec4(v) >> v_2);
  uvec4 v_4 = (v_3 & uvec4(255u));
  uvec4 v_5 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_6 = (uvec4(v_1) >> v_5);
  uint res = dot(v_4, (v_6 & uvec4(255u)));
  return res;
}
void main() {
  prevent_dce = dot4U8Packed_fbed7b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = dot4U8Packed_fbed7b();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot4U8Packed_fbed7b();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'dot' : no matching overloaded function found 
ERROR: 0:22: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:22: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uint prevent_dce;
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = (uvec4(v) >> v_2);
  uvec4 v_4 = (v_3 & uvec4(255u));
  uvec4 v_5 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_6 = (uvec4(v_1) >> v_5);
  uint res = dot(v_4, (v_6 & uvec4(255u)));
  return res;
}
void main() {
  prevent_dce = dot4U8Packed_fbed7b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = dot4U8Packed_fbed7b();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot4U8Packed_fbed7b();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'dot' : no matching overloaded function found 
ERROR: 0:22: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:22: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uint prevent_dce;
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = (uvec4(v) >> v_2);
  uvec4 v_4 = (v_3 & uvec4(255u));
  uvec4 v_5 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_6 = (uvec4(v_1) >> v_5);
  uint res = dot(v_4, (v_6 & uvec4(255u)));
  return res;
}
void main() {
  prevent_dce = dot4U8Packed_fbed7b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = dot4U8Packed_fbed7b();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot4U8Packed_fbed7b();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'dot' : no matching overloaded function found 
ERROR: 0:22: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:22: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
