# v-switch03: line 7: the case selector values must have the same type as the selector expression

entry_point vertex = main;
fn main() -> void {
  var a: i32 = -2;
  switch (a) {
    case 2u:{}
    default: {}
  }
  return;
}

