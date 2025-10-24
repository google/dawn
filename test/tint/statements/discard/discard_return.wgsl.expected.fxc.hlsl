
static bool continue_execution = true;
void f() {
  continue_execution = false;
  if (!(continue_execution)) {
    discard;
  }
}

