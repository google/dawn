
bool get_bool() {
  return true;
}

[numthreads(1, 1, 1)]
void f() {
  bool2 v2 = bool2((get_bool()).xx);
  bool3 v3 = bool3((get_bool()).xxx);
  bool4 v4 = bool4((get_bool()).xxxx);
}

