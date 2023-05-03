#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
shared int a;
int tint_workgroupUniformLoad_a() {
  barrier();
  int result = a;
  barrier();
  return result;
}

shared int b;
int tint_workgroupUniformLoad_b() {
  barrier();
  int result = b;
  barrier();
  return result;
}

void foo() {
  {
    int i = 0;
    while (true) {
      int tint_symbol = i;
      int tint_symbol_1 = tint_workgroupUniformLoad_a();
      if (!((tint_symbol < tint_symbol_1))) {
        break;
      }
      {
      }
      {
        int tint_symbol_2 = i;
        int tint_symbol_3 = tint_workgroupUniformLoad_b();
        i = (tint_symbol_2 + tint_symbol_3);
      }
    }
  }
}

