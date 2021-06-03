void tan_244e2a() {
  float4 res = tan(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  tan_244e2a();
  return;
}

void fragment_main() {
  tan_244e2a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_244e2a();
  return;
}

