[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int tint_workgroupUniformLoad(inout int p) {
  GroupMemoryBarrierWithGroupSync();
  const int result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared int a;
groupshared int b;

void foo() {
  {
    int i = 0;
    while (true) {
      const int tint_symbol = i;
      const int tint_symbol_1 = tint_workgroupUniformLoad(a);
      if (!((tint_symbol < tint_symbol_1))) {
        break;
      }
      {
      }
      {
        const int tint_symbol_2 = i;
        const int tint_symbol_3 = tint_workgroupUniformLoad(b);
        i = (tint_symbol_2 + tint_symbol_3);
      }
    }
  }
}
