@id(1234) override o : bool;

@compute @workgroup_size(1)
fn main() {
  if (o) {
    _ = o;
  }
}
