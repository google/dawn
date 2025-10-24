
static bool t = false;
bool3 m() {
  t = true;
  return bool3((t).xxx);
}

[numthreads(1, 1, 1)]
void f() {
  int3 v = int3(m());
}

