cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};

RWByteAddressBuffer result : register(u1, space1);

struct S {
  uint data[3];
};

static S s = (S)0;

[numthreads(1, 1, 1)]
void main() {
  s.data[constants[0].x] = 0u;
  return;
}
