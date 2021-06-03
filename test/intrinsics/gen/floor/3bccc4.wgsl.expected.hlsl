void floor_3bccc4() {
  float4 res = floor(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  floor_3bccc4();
  return;
}

void fragment_main() {
  floor_3bccc4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  floor_3bccc4();
  return;
}

