struct S {
  float field0;
  float age;
};

static S myvar = (S)0;

void main_1() {
  myvar.age = 42.0f;
  return;
}

void main() {
  main_1();
  return;
}
