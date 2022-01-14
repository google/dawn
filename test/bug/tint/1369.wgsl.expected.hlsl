bug/tint/1369.wgsl:3:3 warning: code is unreachable
  return true;
  ^^^^^^

bug/tint/1369.wgsl:9:9 warning: code is unreachable
    var also_unreachable : bool;
        ^^^^^^^^^^^^^^^^

bool call_discard() {
  if (true) {
    discard;
    return true;
  }
  bool unused;
  return unused;
}

void f() {
  bool v = call_discard();
  bool also_unreachable = false;
  return;
}
