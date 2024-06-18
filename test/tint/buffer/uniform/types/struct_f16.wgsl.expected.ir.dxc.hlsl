SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  S x = u;
  s = x;
}

DXC validation failure:
hlsl.hlsl:3:3: error: unknown type name 'S'
  S x = u;
  ^
hlsl.hlsl:4:3: error: use of undeclared identifier 's'
  s = x;
  ^

