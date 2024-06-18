SKIP: FAILED

void uses_a() {
  int foo = a;
}

void uses_uses_a() {
  uses_a();
}

void uses_b() {
  int foo = b;
}

[numthreads(1, 1, 1)]
void main1() {
  uses_a();
}

[numthreads(1, 1, 1)]
void main2() {
  uses_uses_a();
}

[numthreads(1, 1, 1)]
void main3() {
  uses_b();
}

[numthreads(1, 1, 1)]
void main4() {
}

DXC validation failure:
hlsl.hlsl:2:13: error: use of undeclared identifier 'a'
  int foo = a;
            ^
hlsl.hlsl:10:13: error: use of undeclared identifier 'b'
  int foo = b;
            ^

