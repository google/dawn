SKIP: INVALID


static int t = int(0);
int3 m() {
  t = int(1);
  return int3((t).xxx);
}

void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(9,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
