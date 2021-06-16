SKIP: FAILED



Validation Failure:
float[4] f1() {
  const float tint_symbol[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  return tint_symbol;
}
float[3][4] f2() {
  const float tint_symbol_1[3][4] = {f1(), f1(), f1()};
  return tint_symbol_1;
}
float[2][3][4] f3() {
  const float tint_symbol_2[2][3][4] = {f2(), f2()};
  return tint_symbol_2;
}
[numthreads(1, 1, 1)]
void main() {
  const float a1[4] = f1();
  const float a2[3][4] = f2();
  const float a3[2][3][4] = f3();
  return;
}

tint_gQgfKR:1:14: error: brackets are not allowed here; to declare an array, place the brackets after the name
float[4] f1() {
     ~~~     ^
             [4]
tint_gQgfKR:5:17: error: brackets are not allowed here; to declare an array, place the brackets after the name
float[3][4] f2() {
     ~~~~~~     ^
                [3][4]
tint_gQgfKR:9:20: error: brackets are not allowed here; to declare an array, place the brackets after the name
float[2][3][4] f3() {
     ~~~~~~~~~     ^
                   [2][3][4]

