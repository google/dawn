SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  int i = I;
  int u = (i + 1);
}

DXC validation failure:
hlsl.hlsl:3:11: error: use of undeclared identifier 'I'
  int i = I;
          ^

