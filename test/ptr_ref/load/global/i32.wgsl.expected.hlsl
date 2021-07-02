static int I = 0;

[numthreads(1, 1, 1)]
void main() {
  const int use = (I + 1);
  return;
}
