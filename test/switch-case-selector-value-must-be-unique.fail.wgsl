# v-switch04: line 9: a literal value must not appear more than once in the case selectors for a
# switch statement: '0'

[[stage(vertex)]]
fn main() -> void {
  var a: u32 = 2;
  switch (a) {
    case 10u: {}
    case 10u: {}
    case 10u: {}
    default: {}
  }
  return;
}

