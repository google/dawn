const o : bool = false;

@compute @workgroup_size(1)
fn main() {
  if (o) {
    _ = o;
  }
}
