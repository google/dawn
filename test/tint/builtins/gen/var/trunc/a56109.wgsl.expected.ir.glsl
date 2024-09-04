SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  f16vec2 prevent_dce;
};

f16vec2 prevent_dce;
f16vec2 trunc_a56109() {
  f16vec2 arg_0 = f16vec2(1.5hf);
  f16vec2 res = trunc(arg_0);
  return res;
}
void main() {
  prevent_dce = trunc_a56109();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = trunc_a56109();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec2(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = trunc_a56109();
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
  f16vec2 prevent_dce;
};

f16vec2 prevent_dce;
f16vec2 trunc_a56109() {
  f16vec2 arg_0 = f16vec2(1.5hf);
  f16vec2 res = trunc(arg_0);
  return res;
}
void main() {
  prevent_dce = trunc_a56109();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = trunc_a56109();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec2(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = trunc_a56109();
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
  f16vec2 prevent_dce;
};

f16vec2 prevent_dce;
f16vec2 trunc_a56109() {
  f16vec2 arg_0 = f16vec2(1.5hf);
  f16vec2 res = trunc(arg_0);
  return res;
}
void main() {
  prevent_dce = trunc_a56109();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = trunc_a56109();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec2(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = trunc_a56109();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:21: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
