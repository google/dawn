
void f() {
  int vec3f = int(1);
  int b = vec3f;
  float3 c = (0.0f).xxx;
  float3 d = (0.0f).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

