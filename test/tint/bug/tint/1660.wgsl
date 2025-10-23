override size = 2;

var<workgroup> a : array<f32, size>;

@compute @workgroup_size(1)
fn main() {
    _ = a[0];
}
