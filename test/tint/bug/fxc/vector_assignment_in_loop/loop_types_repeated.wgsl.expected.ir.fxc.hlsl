
[numthreads(1, 1, 1)]
void main() {
  float2 v2f = (0.0f).xx;
  float2 v2f_2 = (0.0f).xx;
  int3 v3i = (int(0)).xxx;
  int3 v3i_2 = (int(0)).xxx;
  uint4 v4u = (0u).xxxx;
  uint4 v4u_2 = (0u).xxxx;
  bool2 v2b = (false).xx;
  bool2 v2b_2 = (false).xx;
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
      float2 v_8 = v2f_2;
      float2 v_9 = i.xx;
      v2f_2 = (((v_9 == float2(int(0), int(1)))) ? (1.0f.xx) : (v_8));
      int3 v_10 = v3i_2;
      int3 v_11 = i.xxx;
      v3i_2 = (((v_11 == int3(int(0), int(1), int(2)))) ? (int(1).xxx) : (v_10));
      uint4 v_12 = v4u_2;
      uint4 v_13 = i.xxxx;
      v4u_2 = (((v_13 == uint4(int(0), int(1), int(2), int(3)))) ? (1u.xxxx) : (v_12));
      bool2 v_14 = v2b_2;
      bool2 v_15 = i.xx;
      v2b_2 = (((v_15 == bool2(int(0), int(1)))) ? (true.xx) : (v_14));
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

