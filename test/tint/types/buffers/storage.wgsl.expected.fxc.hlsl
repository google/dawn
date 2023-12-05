ByteAddressBuffer weights : register(t0);

void main() {
  float a = asfloat(weights.Load(0u));
  return;
}
