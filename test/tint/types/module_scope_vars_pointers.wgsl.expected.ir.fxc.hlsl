SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  float p_ptr = p;
  float w_ptr = w;
  float x = (p_ptr + w_ptr);
  p_ptr = x;
}

