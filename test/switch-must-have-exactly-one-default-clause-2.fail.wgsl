# v-0008: line 6: switch statement must have exactly one default clause

entry_point vertex = main;
fn main() -> void {
  var a: i32 = 2;
  switch (a) {
    case 2: {}
    default: {}
    default: {}
  }
  return;
}

