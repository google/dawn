
static const int v_1[2] = {1, 2};
static int arr[2] = v_1;
void f() {
  int v[2] = arr;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

