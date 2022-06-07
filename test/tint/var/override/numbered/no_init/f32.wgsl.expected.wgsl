@id(1234) override o : f32;

@compute @workgroup_size(1)
fn main() {
  _ = o;
}
