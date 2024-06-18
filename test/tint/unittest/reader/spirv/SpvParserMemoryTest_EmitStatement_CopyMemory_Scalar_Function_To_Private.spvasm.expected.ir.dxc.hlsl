SKIP: FAILED

void main_1() {
  uint x_1 = 0u;
  x_2 = x_1;
}

void main() {
  main_1();
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'x_2'; did you mean 'x_1'?
  x_2 = x_1;
  ^~~
  x_1
hlsl.hlsl:2:8: note: 'x_1' declared here
  uint x_1 = 0u;
       ^

