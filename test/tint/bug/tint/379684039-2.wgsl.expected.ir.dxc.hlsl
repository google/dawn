
static uint idx = 0u;
ByteAddressBuffer _storage : register(t2);
void main() {
  int2 vec = (int(0)).xx;
  {
    while(true) {
      if ((vec.y >= asint(_storage.Load((116u + (idx * 128u)))))) {
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

