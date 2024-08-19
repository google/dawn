SKIP: FAILED


enable clip_distances;

struct VertexOutputs {
  @builtin(position)
  position : vec4<f32>,
  @builtin(clip_distances)
  clipDistance : array<f32, 1>,
}

@vertex
fn main() -> VertexOutputs {
  return VertexOutputs(vec4<f32>(1.0, 2.0, 3.0, 4.0), array<f32, 1>(0.0));
}

Failed to generate: <dawn>/test/tint/extensions/clip_distances/clip_distances_size_1.wgsl:1:8 error: HLSL backend does not support extension 'clip_distances'
enable clip_distances;
       ^^^^^^^^^^^^^^

