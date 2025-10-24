
void f(bool cond) {
  if (cond) {
    discard;
    return;
  }
}

void main() {
  f(false);
}

