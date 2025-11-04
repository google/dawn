
cbuffer cbuffer_value : register(b0, space1) {
  uint4 value[1];
};
RWByteAddressBuffer output : register(u1);
void foo() {
  output.Store(0u, value[0u].x);
}

[numthreads(1, 1, 1)]
void main() {
  foo();
}

