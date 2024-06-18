SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  matrix<float16_t, 2, 4> x = u;
  s = x;
}

DXC validation failure:
hlsl.hlsl:3:31: error: use of undeclared identifier 'u'
  matrix<float16_t, 2, 4> x = u;
                              ^
hlsl.hlsl:4:3: error: use of undeclared identifier 's'
  s = x;
  ^

