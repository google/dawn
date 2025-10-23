@group (0) @binding(1)
var Sampler: sampler;

@group(0) @binding(1)
var randomTexture: texture_external;

@group (0) @binding(2)
var depthTexture: texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
    _ = randomTexture;
    _ = depthTexture;
}


@compute @workgroup_size(1)
fn main2() {
    _ = Sampler;
    _ = depthTexture;
}
