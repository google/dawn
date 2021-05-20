groupshared int i;

[numthreads(1, 1, 1)]
void main() {
  i = 123;
  const int use = (i + 1);
  return;
}

