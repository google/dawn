
void main() {
  int i = int(0);
  i = asint((asuint(i) - asuint(int(1))));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

