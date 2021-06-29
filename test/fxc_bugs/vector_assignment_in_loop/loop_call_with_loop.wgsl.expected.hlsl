void Set_float2(inout float2 vec, int idx, float val) {
  switch(idx) {
    case 0: vec[0] = val; break;
    case 1: vec[1] = val; break;
  }
}
void Set_int3(inout int3 vec, int idx, int val) {
  switch(idx) {
    case 0: vec[0] = val; break;
    case 1: vec[1] = val; break;
    case 2: vec[2] = val; break;
  }
}
void Set_uint4(inout uint4 vec, int idx, uint val) {
  switch(idx) {
    case 0: vec[0] = val; break;
    case 1: vec[1] = val; break;
    case 2: vec[2] = val; break;
    case 3: vec[3] = val; break;
  }
}
void Set_bool2(inout bool2 vec, int idx, bool val) {
  switch(idx) {
    case 0: vec[0] = val; break;
    case 1: vec[1] = val; break;
  }
}
static float2 v2f = float2(0.0f, 0.0f);
static int3 v3i = int3(0, 0, 0);
static uint4 v4u = uint4(0u, 0u, 0u, 0u);
static bool2 v2b = bool2(false, false);

void foo() {
  {
    int i = 0;
    while (true) {
      if (!((i < 2))) {
        break;
      }
      Set_float2(v2f, i, 1.0f);
      Set_int3(v3i, i, 1);
      Set_uint4(v4u, i, 1u);
      Set_bool2(v2b, i, true);
      {
        i = (i + 1);
      }
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  {
    int i = 0;
    while (true) {
      if (!((i < 2))) {
        break;
      }
      foo();
      {
        i = (i + 1);
      }
    }
  }
  return;
}
