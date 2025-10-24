var<private> i : i32 = 0;

@compute @workgroup_size(1)
fn main() {
  i++;
}
