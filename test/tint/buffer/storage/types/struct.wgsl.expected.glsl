SKIP: FAILED

#version 310 es

struct Inner {
  float f;
};

struct S {
  Inner inner;
};

layout(binding = 0, std430) buffer S_1 {
  Inner inner;
} tint_symbol;
layout(binding = 1, std430) buffer S_2 {
  Inner inner;
} tint_symbol_1;
void tint_symbol_2() {
  tint_symbol_1 = tint_symbol;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_2();
  return;
}
Error parsing GLSL shader:
ERROR: 0:18: 'assign' :  cannot convert from 'layout( binding=0 column_major std430) buffer block{layout( column_major std430 offset=0) buffer structure{ global highp float f} inner}' to 'layout( binding=1 column_major std430) buffer block{layout( column_major std430 offset=0) buffer structure{ global highp float f} inner}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



