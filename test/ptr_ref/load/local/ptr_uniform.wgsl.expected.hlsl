cbuffer cbuffer_v : register(b0, space0) {
  uint4 v[1];
};

[numthreads(1, 1, 1)]
void main() {
  const int use = (asint(v[0].x) + 1);
  return;
}
