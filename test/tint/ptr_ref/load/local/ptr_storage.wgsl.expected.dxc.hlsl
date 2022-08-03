RWByteAddressBuffer v : register(u0, space0);

[numthreads(1, 1, 1)]
void main() {
  const int u = (asint(v.Load(0u)) + 1);
  return;
}
