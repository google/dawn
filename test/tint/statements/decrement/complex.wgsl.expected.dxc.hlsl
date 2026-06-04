
RWByteAddressBuffer buffer : register(u0);
static uint v = 0u;
int idx1() {
  v = (v - 1u);
  return int(1);
}

int idx2() {
  v = (v - 1u);
  return int(2);
}

int idx3() {
  v = (v - 1u);
  return int(3);
}

int idx4() {
  v = (v - 1u);
  return int(4);
}

int idx5() {
  v = (v - 1u);
  return int(0);
}

int idx6() {
  v = (v - 1u);
  return int(2);
}

[numthreads(1, 1, 1)]
void main() {
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    int v_1 = idx1();
    int v_2 = idx2();
    uint v_3 = 0u;
    buffer.GetDimensions(v_3);
    uint v_4 = ((v_3 / 64u) - 1u);
    uint v_5 = min(uint(v_1), v_4);
    uint v_6 = (v_5 * 64u);
    uint v_7 = (min(uint(v_2), 3u) * 16u);
    int v_8 = idx3();
    int v_9 = asint((asuint(asint(buffer.Load((((0u + v_6) + v_7) + (min(uint(v_8), 3u) * 4u))))) - asuint(int(1))));
    buffer.Store((((0u + v_6) + v_7) + (min(uint(v_8), 3u) * 4u)), asuint(v_9));
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
        break;
      }
      if ((v < 10u)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        int v_10 = idx4();
        int v_11 = idx5();
        uint v_12 = 0u;
        buffer.GetDimensions(v_12);
        uint v_13 = ((v_12 / 64u) - 1u);
        uint v_14 = min(uint(v_10), v_13);
        uint v_15 = (v_14 * 64u);
        uint v_16 = (min(uint(v_11), 3u) * 16u);
        int v_17 = idx6();
        int v_18 = asint((asuint(asint(buffer.Load((((0u + v_15) + v_16) + (min(uint(v_17), 3u) * 4u))))) - asuint(int(1))));
        buffer.Store((((0u + v_15) + v_16) + (min(uint(v_17), 3u) * 4u)), asuint(v_18));
      }
    }
  }
}

