
static int i = 0;
void main() {
  i = (i + 1);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

