[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int a = 0;
static float b = 0.0f;

void foo(int maybe_zero) {
  a = (a / 1);
  a = (a % 1);
  a = (a / (maybe_zero == 0 ? 1 : maybe_zero));
  a = (a % (maybe_zero == 0 ? 1 : maybe_zero));
  b = (b / 0.0f);
  b = (b % 0.0f);
  b = (b / float(maybe_zero));
  b = (b % float(maybe_zero));
}
