# v-switch01: line 6: switch statement selector expression must be of a scalar integer type

[[stage(vertex)]]
fn main() -> void {
  var a: f32 = 3.14;
  switch (a) {
    default: {}
  }
  return;
}

