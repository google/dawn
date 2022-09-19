[numthreads(1, 1, 1)]
void f() {
  const bool a = true;
  const bool b = false;
  const bool r = (a & b);
  return;
}
