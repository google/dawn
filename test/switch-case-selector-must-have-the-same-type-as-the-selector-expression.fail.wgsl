# v-switch03: line 7: the case selector values must have the same type as the selector expression

[[stage(vertex)]]
fn main() -> void {
  var a: u32 = 2;
  switch (a) {
    case -1:{}
    default: {}
  }
  return;
}

