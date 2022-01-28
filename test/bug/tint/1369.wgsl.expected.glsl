bug/tint/1369.wgsl:3:3 warning: code is unreachable
  return true;
  ^^^^^^

bug/tint/1369.wgsl:9:9 warning: code is unreachable
    var also_unreachable : bool;
        ^^^^^^^^^^^^^^^^

#version 310 es
precision mediump float;

bool call_discard() {
  discard;
  return true;
}

void f() {
  bool v = call_discard();
  bool also_unreachable = false;
}

void main() {
  f();
  return;
}
