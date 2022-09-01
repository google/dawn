[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool4 u = (true).xxxx;

void f() {
  const float4 v = float4(u);
}
