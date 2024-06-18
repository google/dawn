SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  s = (1).xxx;
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 's'
  s = (1).xxx;
  ^

