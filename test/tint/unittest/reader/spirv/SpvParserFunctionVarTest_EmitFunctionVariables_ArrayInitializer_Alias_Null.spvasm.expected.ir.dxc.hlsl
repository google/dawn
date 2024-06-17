SKIP: FAILED

void main_1() {
  strided_arr[2] x_200 = (strided_arr[2])0;
}

void main() {
  main_1();
}

DXC validation failure:
hlsl.hlsl:2:3: error: use of undeclared identifier 'strided_arr'
  strided_arr[2] x_200 = (strided_arr[2])0;
  ^

