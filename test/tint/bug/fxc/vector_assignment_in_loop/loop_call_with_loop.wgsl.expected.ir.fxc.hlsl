
static float2 v2f = (0.0f).xx;
static int3 v3i = (int(0)).xxx;
static uint4 v4u = (0u).xxxx;
static bool2 v2b = (false).xx;
void foo() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(2))) {
      } else {
        break;
      }
      float2 v = v2f;
      float2 v_1 = i.xx;
      v2f = (((v_1 == float2(int(0), int(1)))) ? (1.0f.xx) : (v));
      int3 v_2 = v3i;
      int3 v_3 = i.xxx;
      v3i = (((v_3 == int3(int(0), int(1), int(2)))) ? (int(1).xxx) : (v_2));
      uint4 v_4 = v4u;
      uint4 v_5 = i.xxxx;
      v4u = (((v_5 == uint4(int(0), int(1), int(2), int(3)))) ? (1u.xxxx) : (v_4));
      bool2 v_6 = v2b;
      bool2 v_7 = i.xx;
      v2b = (((v_7 == bool2(int(0), int(1)))) ? (true.xx) : (v_6));
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(2))) {
      } else {
        break;
      }
      foo();
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

