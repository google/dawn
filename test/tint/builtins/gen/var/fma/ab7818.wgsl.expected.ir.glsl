SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  f16vec4 prevent_dce;
};

f16vec4 prevent_dce;
f16vec4 fma_ab7818() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  f16vec4 arg_1 = f16vec4(1.0hf);
  f16vec4 arg_2 = f16vec4(1.0hf);
  f16vec4 res = fma(arg_0, arg_1, arg_2);
  return res;
}
void main() {
  prevent_dce = fma_ab7818();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = fma_ab7818();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec4(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = fma_ab7818();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'fma' : required extension not requested: Possible extensions include:
GL_EXT_gpu_shader5
GL_OES_gpu_shader5
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  f16vec4 prevent_dce;
};

f16vec4 prevent_dce;
f16vec4 fma_ab7818() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  f16vec4 arg_1 = f16vec4(1.0hf);
  f16vec4 arg_2 = f16vec4(1.0hf);
  f16vec4 res = fma(arg_0, arg_1, arg_2);
  return res;
}
void main() {
  prevent_dce = fma_ab7818();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = fma_ab7818();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec4(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = fma_ab7818();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'fma' : required extension not requested: Possible extensions include:
GL_EXT_gpu_shader5
GL_OES_gpu_shader5
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  f16vec4 prevent_dce;
};

f16vec4 prevent_dce;
f16vec4 fma_ab7818() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  f16vec4 arg_1 = f16vec4(1.0hf);
  f16vec4 arg_2 = f16vec4(1.0hf);
  f16vec4 res = fma(arg_0, arg_1, arg_2);
  return res;
}
void main() {
  prevent_dce = fma_ab7818();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = fma_ab7818();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec4(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = fma_ab7818();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'fma' : required extension not requested: Possible extensions include:
GL_EXT_gpu_shader5
GL_OES_gpu_shader5
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
