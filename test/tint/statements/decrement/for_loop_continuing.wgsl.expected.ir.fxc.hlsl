
RWByteAddressBuffer i : register(u0);
void main() {
  {
    while(true) {
      if ((i.Load(0u) < 10u)) {
      } else {
        break;
      }
      {
        i.Store(0u, (i.Load(0u) - 1u));
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

