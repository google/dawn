var<workgroup> i : i32;

@compute @workgroup_size(1)
fn main() {
  i--;
}
