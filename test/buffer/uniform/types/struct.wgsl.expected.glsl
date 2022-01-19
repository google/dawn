SKIP: FAILED

#version 310 es
precision mediump float;

struct Inner {
  float f;
};
struct S {
  Inner inner;
};

layout (binding = 0) uniform S_1 {
  Inner inner;
} u;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  S x = u;
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:17: '=' :  cannot convert from 'layout( binding=0 column_major shared) uniform block{layout( column_major shared) uniform structure{ global mediump float f} inner}' to ' temp structure{ global structure{ global mediump float f} inner}'
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



