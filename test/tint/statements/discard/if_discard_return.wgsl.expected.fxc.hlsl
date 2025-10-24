
static bool continue_execution = true;
void f(bool cond) {
  if (cond) {
    continue_execution = false;
    return;
  }
}

void main() {
  f(false);
  if (!(continue_execution)) {
    discard;
  }
}

