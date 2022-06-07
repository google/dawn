var<private> I : i32;

@compute @workgroup_size(1)
fn main() {
  let i : i32 = I;
  let use : i32 = i + 1;
}
