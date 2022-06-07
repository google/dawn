enable f16;

@fragment
fn main() -> @location(0) vec4<f32> {
  return vec4<f32>(0.100000001, 0.200000003, 0.300000012, 0.400000006);
}
