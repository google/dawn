bug/tint/1369.wgsl:3:3 warning: code is unreachable
  return true;
  ^^^^^^

bug/tint/1369.wgsl:9:5 warning: code is unreachable
    var also_unreachable : bool;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^

static bool tint_discard = false;

bool call_discard() {
  tint_discard = true;
  return false;
  return true;
}

void tint_discard_func() {
  discard;
}

void f() {
  bool v = call_discard();
  if (tint_discard) {
    tint_discard_func();
    return;
  }
  bool also_unreachable = false;
  return;
}
