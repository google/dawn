SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  f16vec2 tint_symbol;
} v;
f16vec2 select_86f9bd() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  f16vec2 arg_1 = f16vec2(1.0hf);
  bool arg_2 = true;
  f16vec2 v_1 = arg_0;
  f16vec2 v_2 = arg_1;
  bool v_3 = arg_2;
  float16_t v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  f16vec2 res = f16vec2(v_4, ((v_3.y) ? (v_2.y) : (v_1.y)));
  return res;
}
void main() {
  v.tint_symbol = select_86f9bd();
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  f16vec2 tint_symbol;
} v;
f16vec2 select_86f9bd() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  f16vec2 arg_1 = f16vec2(1.0hf);
  bool arg_2 = true;
  f16vec2 v_1 = arg_0;
  f16vec2 v_2 = arg_1;
  bool v_3 = arg_2;
  float16_t v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  f16vec2 res = f16vec2(v_4, ((v_3.y) ? (v_2.y) : (v_1.y)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = select_86f9bd();
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:15: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  f16vec2 prevent_dce;
};

layout(location = 0) flat out f16vec2 vertex_main_loc0_Output;
f16vec2 select_86f9bd() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  f16vec2 arg_1 = f16vec2(1.0hf);
  bool arg_2 = true;
  f16vec2 v = arg_0;
  f16vec2 v_1 = arg_1;
  bool v_2 = arg_2;
  float16_t v_3 = ((v_2.x) ? (v_1.x) : (v.x));
  f16vec2 res = f16vec2(v_3, ((v_2.y) ? (v_1.y) : (v.y)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec2(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_86f9bd();
  return tint_symbol;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = v_4.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:18: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
