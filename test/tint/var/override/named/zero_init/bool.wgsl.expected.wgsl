override o : bool = bool();

@compute @workgroup_size(1)
fn main() {
  _ = o;
}
