SKIP: FAILED

void main_1() {
  var_1 = 1u;
  var_1 = 2u;
}

void main() {
  main_1();
}

DXC validation failure:
hlsl.hlsl:2:3: error: use of undeclared identifier 'var_1'
  var_1 = 1u;
  ^
hlsl.hlsl:3:3: error: use of undeclared identifier 'var_1'
  var_1 = 2u;
  ^

