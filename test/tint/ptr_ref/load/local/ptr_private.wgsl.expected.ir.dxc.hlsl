SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  int p = i;
  int u = (p + 1);
}

DXC validation failure:
hlsl.hlsl:3:11: error: use of undeclared identifier 'i'
  int p = i;
          ^

