[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

Texture1D<int4> arg_0 : register(t0, space0);

void d() {
  arg_0.Load(int2(1, 0));
  const float l = 0.141120002f;
}
