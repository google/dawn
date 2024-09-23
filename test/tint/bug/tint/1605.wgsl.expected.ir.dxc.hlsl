
cbuffer cbuffer_b : register(b0) {
  uint4 b[1];
};
bool func_3() {
  {
    int i = int(0);
    while(true) {
      int v = i;
      if ((v < asint(b[0u].x))) {
      } else {
        break;
      }
      {
        int j = int(-1);
        while(true) {
          if ((j == int(1))) {
          } else {
            break;
          }
          return false;
        }
      }
      {
        i = (i + int(1));
      }
      continue;
    }
  }
  return false;
}

[numthreads(1, 1, 1)]
void main() {
  func_3();
}

