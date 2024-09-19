
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
      v2f[i] = 1.0f;
      v3i[i] = int(1);
      v4u[i] = 1u;
      v2b[i] = true;
      v2f_2[i] = 1.0f;
      v3i_2[i] = int(1);
      v4u_2[i] = 1u;
      v2b_2[i] = true;
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

