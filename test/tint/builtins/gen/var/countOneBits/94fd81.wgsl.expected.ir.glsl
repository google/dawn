SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

uvec2 prevent_dce;
uvec2 countOneBits_94fd81() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 res = bitCount(arg_0);
  return res;
}
void main() {
  prevent_dce = countOneBits_94fd81();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = countOneBits_94fd81();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec2(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countOneBits_94fd81();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:14: '=' :  cannot convert from ' global lowp 2-component vector of int' to ' temp highp 2-component vector of uint'
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

uvec2 prevent_dce;
uvec2 countOneBits_94fd81() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 res = bitCount(arg_0);
  return res;
}
void main() {
  prevent_dce = countOneBits_94fd81();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = countOneBits_94fd81();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec2(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countOneBits_94fd81();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:14: '=' :  cannot convert from ' global lowp 2-component vector of int' to ' temp highp 2-component vector of uint'
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

uvec2 prevent_dce;
uvec2 countOneBits_94fd81() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 res = bitCount(arg_0);
  return res;
}
void main() {
  prevent_dce = countOneBits_94fd81();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = countOneBits_94fd81();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec2(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countOneBits_94fd81();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:14: '=' :  cannot convert from ' global lowp 2-component vector of int' to ' temp highp 2-component vector of uint'
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
