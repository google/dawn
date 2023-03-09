#version 310 es
precision highp float;

bool tint_discarded = false;
bool call_discard() {
  tint_discarded = true;
  return true;
}

void f() {
  bool v = call_discard();
  bool also_unreachable = false;
}

void main() {
  f();
  if (tint_discarded) {
    discard;
  }
  return;
}
