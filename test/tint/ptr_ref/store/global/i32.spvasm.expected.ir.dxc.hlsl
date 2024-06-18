SKIP: FAILED

void main_1() {
  I = 123;
  I = 123;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

DXC validation failure:
hlsl.hlsl:2:3: error: use of undeclared identifier 'I'
  I = 123;
  ^
hlsl.hlsl:3:3: error: use of undeclared identifier 'I'
  I = 123;
  ^

