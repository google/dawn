
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void main() {
  uint q = 0u;
  uint v = 0u;
  s.GetDimensions(v);
  uint v_1 = ((v / 4u) - 1u);
  uint v_2 = (min(uint(int(0)), v_1) * 4u);
  uint2 v_3[2] = {uint2(1u, 2u), uint2(3u, 4u)};
  s.Store((0u + v_2), v_3[min(q, 1u)].x);
}

