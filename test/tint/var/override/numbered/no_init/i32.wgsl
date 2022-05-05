// flags: --overrides WGSL_SPEC_CONSTANT_1234=0
@id(1234) override o : i32;

@stage(compute) @workgroup_size(1)
fn main() {
    _ = o;
}
