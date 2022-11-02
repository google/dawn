bug/tint/1369.wgsl:3:3 warning: code is unreachable
  return true;
  ^^^^^^

bug/tint/1369.wgsl:9:5 warning: code is unreachable
    var also_unreachable : bool;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^

fn call_discard() -> bool {
  discard;
  return true;
}

@fragment
fn f() {
  var v = call_discard();
  var also_unreachable : bool;
}
