int foo() {
  return 1;
}

void main() {
  float arr[4] = (float[4])0;
  const int a_save = foo();
  {
    for(; ; ) {
      const float x = arr[a_save];
      break;
    }
  }
  return;
}
