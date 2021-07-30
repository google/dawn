struct S {
  float field0;
  float age;
};
struct S_1 {
  float field0;
  float ancientness;
};

static S myvar = (S)0;
static S_1 myvar2 = (S_1)0;

void main_1() {
  myvar.age = 42.0f;
  myvar2.ancientness = 420.0f;
  return;
}

void main() {
  main_1();
  return;
}
