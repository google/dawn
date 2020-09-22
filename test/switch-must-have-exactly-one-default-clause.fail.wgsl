# v-0008: line 6: switch statement must have exactly one default clause

[[stage(vertex)]]
fn main() -> void {
  var a: i32 = 2;
  switch (a) {
    case 2: {}
  }
  return;
}

