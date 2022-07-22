const o : i32 = 0i;

@compute @workgroup_size(1)
fn main() {
  if ((o == 1)) {
    _ = o;
  }
}
