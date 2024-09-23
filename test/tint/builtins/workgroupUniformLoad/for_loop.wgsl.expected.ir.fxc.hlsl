
groupshared int a;
groupshared int b;
void foo() {
  {
    int i = int(0);
    while(true) {
      int v = i;
      GroupMemoryBarrierWithGroupSync();
      int v_1 = a;
      GroupMemoryBarrierWithGroupSync();
      if ((v < v_1)) {
      } else {
        break;
      }
      {
        GroupMemoryBarrierWithGroupSync();
        int v_2 = b;
        GroupMemoryBarrierWithGroupSync();
        i = (i + v_2);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

