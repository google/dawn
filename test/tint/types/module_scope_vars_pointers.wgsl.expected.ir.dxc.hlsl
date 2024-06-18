SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  float p_ptr = p;
  float w_ptr = w;
  float x = (p_ptr + w_ptr);
  p_ptr = x;
}

DXC validation failure:
hlsl.hlsl:3:17: error: use of undeclared identifier 'p'
  float p_ptr = p;
                ^
hlsl.hlsl:4:17: error: use of undeclared identifier 'w'
  float w_ptr = w;
                ^

