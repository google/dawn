SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 0) buffer S_1 {
  int a[];
} G;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  uint tint_symbol_2 = 0u;
  G.GetDimensions(tint_symbol_2);
  uint tint_symbol_3 = ((tint_symbol_2 - 0u) / 4u);
  uint l1 = tint_symbol_3;
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:12: 'GetDimensions' : no such field in structure 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



