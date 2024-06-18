SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  vector<float16_t, 4> x = u;
  s = x;
}

DXC validation failure:
hlsl.hlsl:3:28: error: use of undeclared identifier 'u'
  vector<float16_t, 4> x = u;
                           ^
hlsl.hlsl:4:3: error: use of undeclared identifier 's'
  s = x;
  ^

