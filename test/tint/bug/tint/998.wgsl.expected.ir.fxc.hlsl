SKIP: FAILED

struct S {
  uint data[3];
};


cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};
RWByteAddressBuffer result : register(u1, space1);
static S s = (S)0;
[numthreads(1, 1, 1)]
void main() {
  s.data[constants[0u].x] = 0u;
}

FXC validation failure:
<scrubbed_path>(13,3-25): error X3500: array reference cannot be used as an l-value; not natively addressable

