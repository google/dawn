
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void main() {
  uint v_1 = 0u;
  v.GetDimensions(v_1);
  bool v_2 = (v_1 < 64u);
  uint v_3 = ((v_2) ? (0u) : (0u));
  v.Store(((0u + (v_3 * 1u)) + (min(4u, ((((v_2) ? (4u) : (64u)) / 4u) - 1u)) * 4u)), 4u);
}

