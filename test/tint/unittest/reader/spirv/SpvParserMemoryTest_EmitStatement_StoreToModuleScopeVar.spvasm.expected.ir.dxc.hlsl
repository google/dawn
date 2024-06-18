SKIP: FAILED

void main_1() {
  x_1 = 42u;
}

void main() {
  main_1();
}

DXC validation failure:
hlsl.hlsl:2:3: error: use of undeclared identifier 'x_1'
  x_1 = 42u;
  ^

