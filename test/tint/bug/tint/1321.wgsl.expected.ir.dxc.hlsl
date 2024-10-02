
int foo() {
  return int(1);
}

void main() {
  float arr[4] = (float[4])0;
  {
    int v = foo();
    while(true) {
      float x = arr[v];
      break;
    }
  }
}

