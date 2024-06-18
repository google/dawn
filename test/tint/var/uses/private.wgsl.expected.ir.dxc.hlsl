SKIP: FAILED

void uses_a() {
  a = (a + 1);
}

void uses_b() {
  b = (b * 2);
}

void uses_a_and_b() {
  b = a;
}

void no_uses() {
}

void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}

[numthreads(1, 1, 1)]
void main1() {
  a = 42;
  uses_a();
}

[numthreads(1, 1, 1)]
void main2() {
  b = 7;
  uses_b();
}

[numthreads(1, 1, 1)]
void main3() {
  outer();
  no_uses();
}

[numthreads(1, 1, 1)]
void main4() {
  no_uses();
}

DXC validation failure:
hlsl.hlsl:2:3: error: use of undeclared identifier 'a'
  a = (a + 1);
  ^
hlsl.hlsl:2:8: error: use of undeclared identifier 'a'
  a = (a + 1);
       ^
hlsl.hlsl:6:3: error: use of undeclared identifier 'b'
  b = (b * 2);
  ^
hlsl.hlsl:6:8: error: use of undeclared identifier 'b'
  b = (b * 2);
       ^
hlsl.hlsl:10:3: error: use of undeclared identifier 'b'
  b = a;
  ^
hlsl.hlsl:10:7: error: use of undeclared identifier 'a'
  b = a;
      ^
hlsl.hlsl:17:3: error: use of undeclared identifier 'a'
  a = 0;
  ^
hlsl.hlsl:26:3: error: use of undeclared identifier 'a'
  a = 42;
  ^
hlsl.hlsl:32:3: error: use of undeclared identifier 'b'
  b = 7;
  ^

