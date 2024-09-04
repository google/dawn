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
float16_t mix_38cbbb() {
  float16_t arg_0 = 1.0hf;
  float16_t arg_1 = 1.0hf;
  float16_t arg_2 = 1.0hf;
  float16_t res = mix(arg_0, arg_1, arg_2);
  return res;
}
void main() {
  prevent_dce = mix_38cbbb();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = mix_38cbbb();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0hf);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = mix_38cbbb();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:23: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:23: '' : compilation terminated 
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
float16_t mix_38cbbb() {
  float16_t arg_0 = 1.0hf;
  float16_t arg_1 = 1.0hf;
  float16_t arg_2 = 1.0hf;
  float16_t res = mix(arg_0, arg_1, arg_2);
  return res;
}
void main() {
  prevent_dce = mix_38cbbb();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = mix_38cbbb();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0hf);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = mix_38cbbb();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:24: 'main' : function already has a body 
ERROR: 0:24: '' : compilation terminated 
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
float16_t mix_38cbbb() {
  float16_t arg_0 = 1.0hf;
  float16_t arg_1 = 1.0hf;
  float16_t arg_2 = 1.0hf;
  float16_t res = mix(arg_0, arg_1, arg_2);
  return res;
}
void main() {
  prevent_dce = mix_38cbbb();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = mix_38cbbb();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0hf);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = mix_38cbbb();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:23: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:23: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
