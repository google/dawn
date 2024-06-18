SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  s = 3;
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 's'
  s = 3;
  ^

