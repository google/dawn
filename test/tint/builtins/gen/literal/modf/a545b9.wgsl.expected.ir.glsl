SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct modf_result_vec2_f16 {
  f16vec2 fract;
  f16vec2 whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_a545b9() {
  modf_result_vec2_f16 res = modf_result_vec2_f16(f16vec2(-0.5hf), f16vec2(-1.0hf));
}
void main() {
  modf_a545b9();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_a545b9();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_a545b9();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct modf_result_vec2_f16 {
  f16vec2 fract;
  f16vec2 whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_a545b9() {
  modf_result_vec2_f16 res = modf_result_vec2_f16(f16vec2(-0.5hf), f16vec2(-1.0hf));
}
void main() {
  modf_a545b9();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_a545b9();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_a545b9();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:23: 'main' : function already has a body 
ERROR: 0:23: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct modf_result_vec2_f16 {
  f16vec2 fract;
  f16vec2 whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_a545b9() {
  modf_result_vec2_f16 res = modf_result_vec2_f16(f16vec2(-0.5hf), f16vec2(-1.0hf));
}
void main() {
  modf_a545b9();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_a545b9();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_a545b9();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
