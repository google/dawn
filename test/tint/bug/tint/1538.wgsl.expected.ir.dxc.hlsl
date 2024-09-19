
RWByteAddressBuffer buf : register(u1);
int g() {
  return int(0);
}

int f() {
  {
    while(true) {
      g();
      break;
    }
  }
  int o = g();
  return int(0);
}

[numthreads(1, 1, 1)]
void main() {
  {
    while(true) {
      if ((buf.Load(0u) == 0u)) {
        break;
      }
      int s = f();
      buf.Store(0u, 0u);
      {
      }
      continue;
    }
  }
}

