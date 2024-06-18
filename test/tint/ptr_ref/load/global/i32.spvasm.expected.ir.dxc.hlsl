SKIP: FAILED

void main_1() {
  int x_11 = (I + 1);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

DXC validation failure:
hlsl.hlsl:2:15: error: use of undeclared identifier 'I'
  int x_11 = (I + 1);
              ^

