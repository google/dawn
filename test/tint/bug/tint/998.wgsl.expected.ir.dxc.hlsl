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
  uint v = constants[0u].x;
  s.data[v] = 0u;
}

