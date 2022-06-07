@compute
@workgroup_size(1)
fn main() {
    var v = select(true & true, true, false);
}
