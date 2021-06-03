void max_4883ac() {
  float3 res = max(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  max_4883ac();
  return;
}

void fragment_main() {
  max_4883ac();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_4883ac();
  return;
}

