SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  float16_t prevent_dce;
};

float16_t prevent_dce;
float16_t exp_c18fe9() {
  float16_t arg_0 = 1.0hf;
  float16_t res = exp(arg_0);
  return res;
}
void main() {
  prevent_dce = exp_c18fe9();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = exp_c18fe9();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0hf);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = exp_c18fe9();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:21: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  float16_t prevent_dce;
};

float16_t prevent_dce;
float16_t exp_c18fe9() {
  float16_t arg_0 = 1.0hf;
  float16_t res = exp(arg_0);
  return res;
}
void main() {
  prevent_dce = exp_c18fe9();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = exp_c18fe9();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0hf);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = exp_c18fe9();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'main' : function already has a body 
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  float16_t prevent_dce;
};

float16_t prevent_dce;
float16_t exp_c18fe9() {
  float16_t arg_0 = 1.0hf;
  float16_t res = exp(arg_0);
  return res;
}
void main() {
  prevent_dce = exp_c18fe9();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = exp_c18fe9();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0hf);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = exp_c18fe9();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:21: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
