
int foo() {
  return int(1);
}

void main() {
  float arr[4] = (float[4])0;
  {
    uint v = min(uint(foo()), 3u);
    while(true) {
      float x = arr[v];
      break;
    }
  }
}

