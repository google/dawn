SKIP: FAILED

#version 310 es

struct Buf1 {
  int result;
};

struct Buf0 {
  uint values[];
};

Buf1 x_4;
Buf0 x_7;
void main_1() {
  uint i = 0u;
  x_4.result = 1;
  i = 0u;
  {
    while(true) {
      uint x_33 = i;
      if ((x_33 < 512u)) {
      } else {
        break;
      }
      uint x_36 = i;
      uint x_39 = x_7.values[(x_36 * 2u)];
      uint x_40 = i;
      if ((x_39 != x_40)) {
        x_4.result = 0;
      }
      {
        uint x_45 = i;
        i = (x_45 + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:8: '' : array size required 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
