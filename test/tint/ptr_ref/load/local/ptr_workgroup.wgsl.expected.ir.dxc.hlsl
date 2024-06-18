SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  i = 123;
  int p = i;
  int u = (p + 1);
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'i'
  i = 123;
  ^
hlsl.hlsl:4:11: error: use of undeclared identifier 'i'
  int p = i;
          ^

