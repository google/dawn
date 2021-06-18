cbuffer cbuffer_v : register(b0, space0) {
  uint4 v[1];
};

[numthreads(1, 1, 1)]
void main() {
  const int scalar_offset = (0u) / 4;
  const int use = (asint(v[scalar_offset / 4][scalar_offset % 4]) + 1);
  return;
}
