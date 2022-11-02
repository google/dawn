bug/tint/1369.wgsl:3:3 warning: code is unreachable
  return true;
  ^^^^^^

bug/tint/1369.wgsl:9:5 warning: code is unreachable
    var also_unreachable : bool;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^

#version 310 es
precision mediump float;

bool tint_discard = false;
bool call_discard() {
  tint_discard = true;
  return false;
  return true;
}

void f() {
  bool v = call_discard();
  if (tint_discard) {
    return;
  }
  bool also_unreachable = false;
}

void tint_discard_func() {
  discard;
}

void main() {
  f();
  if (tint_discard) {
    tint_discard_func();
    return;
  }
  return;
}
