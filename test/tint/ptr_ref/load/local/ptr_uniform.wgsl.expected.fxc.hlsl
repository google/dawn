
cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
[numthreads(1, 1, 1)]
void main() {
  int u = asint((asuint(asint(v[0u].x)) + asuint(int(1))));
}

