
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void main() {
  uint q = 0u;
  uint v = 0u;
  s.GetDimensions(v);
  uint v_1[2][2][2] = {{{0u, 1u}, {2u, 3u}}, {{4u, 5u}, {6u, 7u}}};
  s.Store((0u + (min(0u, ((v / 4u) - 1u)) * 4u)), v_1[min(q, 1u)][min(q, 1u)][min(q, 1u)]);
}

