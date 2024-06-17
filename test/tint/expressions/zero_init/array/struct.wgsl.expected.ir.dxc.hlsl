SKIP: FAILED

void f() {
  S[4] v = (S[4])0;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:2:3: error: use of undeclared identifier 'S'
  S[4] v = (S[4])0;
  ^

