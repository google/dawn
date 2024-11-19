[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint idx = 0u;

ByteAddressBuffer _storage : register(t2);

void main() {
  int2 vec = (0).xx;
  while (true) {
    if ((vec.y >= asint(_storage.Load((((128u * idx) + 112u) + 4u))))) {
      break;
    }
  }
}
