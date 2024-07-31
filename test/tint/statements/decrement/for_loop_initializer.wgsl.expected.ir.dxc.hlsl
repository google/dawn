
RWByteAddressBuffer i : register(u0);
void main() {
  {
    i.Store(0u, (i.Load(0u) - 1u));
    while(true) {
      if ((i.Load(0u) < 10u)) {
      } else {
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

