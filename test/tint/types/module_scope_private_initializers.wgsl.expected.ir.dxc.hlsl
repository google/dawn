SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  float x = (a + b);
}

DXC validation failure:
hlsl.hlsl:3:14: error: use of undeclared identifier 'a'
  float x = (a + b);
             ^
hlsl.hlsl:3:18: error: use of undeclared identifier 'b'
  float x = (a + b);
                 ^

