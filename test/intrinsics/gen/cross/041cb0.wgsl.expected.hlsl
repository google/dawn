void cross_041cb0() {
  float3 res = cross(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  cross_041cb0();
  return;
}

void fragment_main() {
  cross_041cb0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cross_041cb0();
  return;
}

