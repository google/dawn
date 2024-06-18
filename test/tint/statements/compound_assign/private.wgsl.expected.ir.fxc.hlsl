SKIP: FAILED

void foo() {
  a = (a / 2);
  b = (b * float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  c = (c * 2.0f);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

