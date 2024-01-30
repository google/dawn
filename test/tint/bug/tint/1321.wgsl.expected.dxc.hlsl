int foo() {
  return 1;
}

void main() {
  float arr[4] = (float[4])0;
  int a_save = foo();
  {
    for(; ; ) {
      float x = arr[a_save];
      break;
    }
  }
  return;
}
