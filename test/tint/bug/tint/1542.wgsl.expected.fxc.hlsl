
cbuffer cbuffer_u_input : register(b0) {
  uint4 u_input[1];
};
[numthreads(1, 1, 1)]
void main() {
  int3 temp = (asint(u_input[0u].xyz) << ((0u).xxx & (31u).xxx));
}

