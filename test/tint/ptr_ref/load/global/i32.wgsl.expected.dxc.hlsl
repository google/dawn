static int I = 0;

[numthreads(1, 1, 1)]
void main() {
  const int u = (I + 1);
  return;
}
