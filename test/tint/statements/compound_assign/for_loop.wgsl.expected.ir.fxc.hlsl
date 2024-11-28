
RWByteAddressBuffer v : register(u0);
static uint i = 0u;
int idx1() {
  i = (i + 1u);
  return int(1);
}

int idx2() {
  i = (i + 2u);
  return int(1);
}

int idx3() {
  i = (i + 3u);
  return int(1);
}

void foo() {
  float a[4] = (float[4])0;
  {
    uint v_1 = min(uint(idx1()), 3u);
    a[v_1] = (a[v_1] * 2.0f);
    while(true) {
      uint v_2 = min(uint(idx2()), 3u);
      if ((a[v_2] < 10.0f)) {
      } else {
        break;
      }
      {
        uint v_3 = min(uint(idx3()), 3u);
        a[v_3] = (a[v_3] + 1.0f);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

