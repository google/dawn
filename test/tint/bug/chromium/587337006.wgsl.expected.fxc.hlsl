
cbuffer cbuffer_tint_immediate_data : register(b0) {
  uint4 tint_immediate_data[1];
};
void main_inner(uint3 n) {
  int i = int(0);
  bool3 v = (true).xxx;
  bool b = !(bool3((all(v)).xxx)[i]);
  {
    while(true) {
      break;
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  main_inner(uint3(tint_immediate_data[0u].x, tint_immediate_data[0u].y, tint_immediate_data[0u].z));
}

