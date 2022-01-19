SKIP: FAILED

#version 310 es
precision mediump float;

struct Inner {
  float f;
};
struct S {
  Inner inner;
};

layout (binding = 0) buffer S_1 {
  Inner inner;
} tint_symbol;
layout (binding = 1) buffer S_2 {
  Inner inner;
} tint_symbol_1;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol_2() {
  tint_symbol_1 = tint_symbol;
  return;
}
void main() {
  tint_symbol_2();
}


Error parsing GLSL shader:
ERROR: 0:20: 'assign' :  cannot convert from 'layout( binding=0 column_major shared) buffer block{layout( column_major shared) buffer structure{ global mediump float f} inner}' to 'layout( binding=1 column_major shared) buffer block{layout( column_major shared) buffer structure{ global mediump float f} inner}'
ERROR: 0:20: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



