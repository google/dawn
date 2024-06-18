SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  float16_t x = u;
  s = x;
}

DXC validation failure:
hlsl.hlsl:3:17: error: use of undeclared identifier 'u'
  float16_t x = u;
                ^
hlsl.hlsl:4:3: error: use of undeclared identifier 's'
  s = x;
  ^

