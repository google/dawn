SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  I = 123;
  I = 123;
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'I'
  I = 123;
  ^
hlsl.hlsl:4:3: error: use of undeclared identifier 'I'
  I = 123;
  ^

