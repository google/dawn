SKIP: FAILED

#version 310 es
precision mediump float;

struct Buf1 {
  int result;
};
struct Buf0 {
  uint values[];
};

layout (binding = 1) buffer Buf1_1 {
  int result;
} x_4;
layout (binding = 0) buffer Buf0_1 {
  uint values[];
} x_7;

void main_1() {
  uint i = 0u;
  x_4.result = 1;
  i = 0u;
  {
    for(; (i < 512u); i = (i + uint(1))) {
      uint x_39 = x_7.values[(i * 2u)];
      if ((x_39 != i)) {
        x_4.result = 0;
      }
    }
  }
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:8: '' : array size required 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



