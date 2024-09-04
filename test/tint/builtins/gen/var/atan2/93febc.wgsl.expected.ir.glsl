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
f16vec2 atan2_93febc() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  f16vec2 arg_1 = f16vec2(1.0hf);
  f16vec2 res = atan(arg_0, arg_1);
  return res;
}
void main() {
  prevent_dce = atan2_93febc();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = atan2_93febc();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec2(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = atan2_93febc();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
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
f16vec2 atan2_93febc() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  f16vec2 arg_1 = f16vec2(1.0hf);
  f16vec2 res = atan(arg_0, arg_1);
  return res;
}
void main() {
  prevent_dce = atan2_93febc();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = atan2_93febc();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec2(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = atan2_93febc();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:23: 'main' : function already has a body 
ERROR: 0:23: '' : compilation terminated 
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
f16vec2 atan2_93febc() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  f16vec2 arg_1 = f16vec2(1.0hf);
  f16vec2 res = atan(arg_0, arg_1);
  return res;
}
void main() {
  prevent_dce = atan2_93febc();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = atan2_93febc();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec2(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = atan2_93febc();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
