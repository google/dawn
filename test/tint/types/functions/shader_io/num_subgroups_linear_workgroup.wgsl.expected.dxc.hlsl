
RWByteAddressBuffer output : register(u0);
void main_inner(uint num_subgroups) {
  uint v = 0u;
  output.GetDimensions(v);
  output.Store((0u + (min(num_subgroups, ((v / 4u) - 1u)) * 4u)), num_subgroups);
}

[numthreads(64, 1, 1)]
void main() {
  main_inner(((63u + WaveGetLaneCount()) / WaveGetLaneCount()));
}

