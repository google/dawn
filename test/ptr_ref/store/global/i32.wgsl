var<private> I : i32;

[[stage(compute)]]
fn main() {
  I = 123; // constant
  I = 100 + 20 + 3; // dynamic
}
