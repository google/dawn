SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  vec2 prevent_dce;
};

vec2 prevent_dce;
vec2 sign_d065d8() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = sign(arg_0);
  return res;
}
void main() {
  prevent_dce = sign_d065d8();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = sign_d065d8();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec2(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = sign_d065d8();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:20: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:20: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  vec2 prevent_dce;
};

vec2 prevent_dce;
vec2 sign_d065d8() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = sign(arg_0);
  return res;
}
void main() {
  prevent_dce = sign_d065d8();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = sign_d065d8();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec2(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = sign_d065d8();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:21: 'main' : function already has a body 
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  vec2 prevent_dce;
};

vec2 prevent_dce;
vec2 sign_d065d8() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = sign(arg_0);
  return res;
}
void main() {
  prevent_dce = sign_d065d8();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = sign_d065d8();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec2(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = sign_d065d8();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:20: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:20: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
