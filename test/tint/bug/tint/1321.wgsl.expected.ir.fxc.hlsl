
int foo() {
  return int(1);
}

void main() {
  float arr[4] = (float[4])0;
  {
    float v = arr[foo()];
    while(true) {
      float x = v;
      break;
    }
  }
}

