SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  int c = 0;
  int d = 0;
  s = (c + d);
}

DXC validation failure:
hlsl.hlsl:5:3: error: use of undeclared identifier 's'
  s = (c + d);
  ^

