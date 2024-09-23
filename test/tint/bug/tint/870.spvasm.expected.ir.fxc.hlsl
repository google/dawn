
ByteAddressBuffer sspp962805860buildInformation : register(t2);
typedef int ary_ret[6];
ary_ret v(uint offset) {
  int a[6] = (int[6])0;
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 6u)) {
        break;
      }
      a[v_2] = asint(sspp962805860buildInformation.Load((offset + (v_2 * 4u))));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  int v_3[6] = a;
  return v_3;
}

void main_1() {
  int orientation[6] = (int[6])0;
  int v_4[6] = v(36u);
  int x_23[6] = v_4;
  orientation[int(0)] = x_23[0u];
  int v_5[6] = v_4;
  orientation[int(1)] = v_5[1u];
  int v_6[6] = v_4;
  orientation[int(2)] = v_6[2u];
  int v_7[6] = v_4;
  orientation[int(3)] = v_7[3u];
  int v_8[6] = v_4;
  orientation[int(4)] = v_8[4u];
  int v_9[6] = v_4;
  orientation[int(5)] = v_9[5u];
}

void main() {
  main_1();
}

