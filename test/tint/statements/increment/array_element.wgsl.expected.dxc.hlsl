
RWByteAddressBuffer a : register(u0);
[numthreads(1, 1, 1)]
void main() {
  uint v = 0u;
  a.GetDimensions(v);
  uint v_1 = (min(1u, ((v / 4u) - 1u)) * 4u);
  a.Store((0u + v_1), (a.Load((0u + v_1)) + 1u));
}

