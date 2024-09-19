
RWTexture2D<int4> tex : register(u2);
void foo() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(3))) {
      } else {
        break;
      }
      {
        tex[(int(0)).xx] = (int(0)).xxxx;
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

