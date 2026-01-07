
RWByteAddressBuffer buffer : register(u0);
void main_inner(uint sg_id, uint sg_size) {
  uint v = 0u;
  buffer.GetDimensions(v);
  buffer.Store((0u + (min(sg_id, ((v / 4u) - 1u)) * 4u)), sg_size);
}

[numthreads(32, 1, 1)]
[WaveSize(32)]
void main() {
  main_inner(WaveGetLaneIndex(), WaveGetLaneCount());
}

