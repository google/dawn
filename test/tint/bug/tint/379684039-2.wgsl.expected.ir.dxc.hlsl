
static uint idx = 0u;
ByteAddressBuffer _storage : register(t2);
void main() {
  int2 vec = (int(0)).xx;
  {
    while(true) {
      uint v = 0u;
      _storage.GetDimensions(v);
      if ((vec.y >= asint(_storage.Load((116u + (min(idx, ((v / 128u) - 1u)) * 128u)))))) {
        break;
      }
      {
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

