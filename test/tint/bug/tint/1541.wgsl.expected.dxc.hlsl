[numthreads(1, 1, 1)]
void main() {
  const bool a = true;
  bool v = (false ? true : (a & true));
  return;
}
