int main0() {
  return 1;
}
uint main1() {
  return 1u;
}
float main2() {
  return 1.0f;
}
float4 main3() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}
float16_t main4() {
  return float16_t(2.25h);
}
vector<float16_t, 3> main5() {
  return vector<float16_t, 3>(float16_t(3.0h), float16_t(5.0h), float16_t(8.0h));
}
