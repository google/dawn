const
H=1;

@compute @workgroup_size(1)
fn main() {
    let a = H;
}
