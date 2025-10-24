
static bool t = false;
bool4 m() {
  t = true;
  return bool4((t).xxxx);
}

[numthreads(1, 1, 1)]
void f() {
  int4 v = int4(m());
}

