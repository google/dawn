# v-switch05: line 9: a fallthrough statement must not appear as the last statement in last clause
# of a switch

[[stage(vertex)]]
fn main() -> void {
  var a: i32 = -2;
  switch (a) {
    default: {
      fallthrough;
    }
  }
  return;
}
