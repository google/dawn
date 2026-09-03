
ByteAddressBuffer weights : register(t0);
void main() {
  uint v = 0u;
  weights.GetDimensions(v);
  float a = asfloat(weights.Load((0u + (min(0u, ((v / 4u) - 1u)) * 4u))));
}

