[numthreads(1, 1, 1)]
void f() {
  const bool3 a = bool3(true, true, false);
  const bool3 b = bool3(true, false, true);
  const bool3 r = (bool3(true, true, false) & bool3(true, false, true));
  return;
}
