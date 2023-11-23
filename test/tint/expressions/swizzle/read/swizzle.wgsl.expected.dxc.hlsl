[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  float3 val[3];
};

void a() {
  int4 a_1 = (0).xxxx;
  const int b = a_1.x;
  const int4 c = a_1.zzyy;
  S d = (S)0;
  const float3 e = d.val[2].yzx;
}
