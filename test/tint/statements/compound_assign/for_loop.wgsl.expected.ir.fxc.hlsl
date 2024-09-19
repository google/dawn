
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
    float v_1 = a[idx1()];
    v_1 = (v_1 * 2.0f);
    while(true) {
      if ((a[idx2()] < 10.0f)) {
      } else {
        break;
      }
      {
        float v_2 = a[idx3()];
        v_2 = (v_2 + 1.0f);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

