SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uint prevent_dce;
uint pack4xU8Clamp_6b8c1b() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 v = arg_0;
  uvec4 v_1 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_2 = uvec4(0u);
  uvec4 v_3 = (clamp(v, v_2, uvec4(255u)) << v_1);
  uint res = dot(v_3, uvec4(1u));
  return res;
}
void main() {
  prevent_dce = pack4xU8Clamp_6b8c1b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = pack4xU8Clamp_6b8c1b();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = pack4xU8Clamp_6b8c1b();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'dot' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uint prevent_dce;
uint pack4xU8Clamp_6b8c1b() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 v = arg_0;
  uvec4 v_1 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_2 = uvec4(0u);
  uvec4 v_3 = (clamp(v, v_2, uvec4(255u)) << v_1);
  uint res = dot(v_3, uvec4(1u));
  return res;
}
void main() {
  prevent_dce = pack4xU8Clamp_6b8c1b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = pack4xU8Clamp_6b8c1b();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = pack4xU8Clamp_6b8c1b();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'dot' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uint prevent_dce;
uint pack4xU8Clamp_6b8c1b() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 v = arg_0;
  uvec4 v_1 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_2 = uvec4(0u);
  uvec4 v_3 = (clamp(v, v_2, uvec4(255u)) << v_1);
  uint res = dot(v_3, uvec4(1u));
  return res;
}
void main() {
  prevent_dce = pack4xU8Clamp_6b8c1b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = pack4xU8Clamp_6b8c1b();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = pack4xU8Clamp_6b8c1b();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'dot' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
