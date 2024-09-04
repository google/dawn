SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct frexp_result_vec2_f16 {
  f16vec2 fract;
  ivec2 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_5f47bf() {
  frexp_result_vec2_f16 res = frexp_result_vec2_f16(f16vec2(0.5hf), ivec2(1));
}
void main() {
  frexp_5f47bf();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5f47bf();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5f47bf();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct frexp_result_vec2_f16 {
  f16vec2 fract;
  ivec2 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_5f47bf() {
  frexp_result_vec2_f16 res = frexp_result_vec2_f16(f16vec2(0.5hf), ivec2(1));
}
void main() {
  frexp_5f47bf();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5f47bf();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5f47bf();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:23: 'main' : function already has a body 
ERROR: 0:23: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct frexp_result_vec2_f16 {
  f16vec2 fract;
  ivec2 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_5f47bf() {
  frexp_result_vec2_f16 res = frexp_result_vec2_f16(f16vec2(0.5hf), ivec2(1));
}
void main() {
  frexp_5f47bf();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5f47bf();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5f47bf();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
