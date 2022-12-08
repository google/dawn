[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  {
    int vec3f_1 = 1;
    int b = vec3f_1;
  }
  float3 c = (0.0f).xxx;
  float3 d = (0.0f).xxx;
}
