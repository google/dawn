
RWByteAddressBuffer S : register(u0);
void v(uint offset, int obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 4u)) {
        break;
      }
      S.Store((offset + (v_1 * 4u)), asuint(obj[v_1]));
      {
        idx = (idx + 1u);
      }
    }
  }
}

void func() {
  int v_2[4] = (int[4])0;
  v(0u, v_2);
}

[numthreads(1, 1, 1)]
void main() {
  func();
}

