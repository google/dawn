@group(0) @binding(0)
var<storage, read_write> value: vec2u;

@compute @workgroup_size(1)
fn main() {
  value = firstLeadingBit(firstLeadingBit(firstLeadingBit(firstLeadingBit(value))));
}
