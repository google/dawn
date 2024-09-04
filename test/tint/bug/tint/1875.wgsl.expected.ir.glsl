SKIP: FAILED

#version 310 es

struct Outputs {
  uint data[];
};

uint count = 0u;
Outputs outputs;
void push_output(uint value) {
  outputs.data[count] = value;
  count = (count + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint a = 0u;
  uint b = 10u;
  uint c = 4294967294u;
  a = (a + 1u);
  b = (b + 1u);
  c = (c + 1u);
  push_output(a);
  push_output(b);
  push_output(c);
}
error: Error parsing GLSL shader:
ERROR: 0:4: '' : array size required 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
