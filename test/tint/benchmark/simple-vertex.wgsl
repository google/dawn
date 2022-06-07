struct Input {
  @location(0) position: vec4<f32>,
};

struct Output {
  @builtin(position) position : vec4<f32>,
};

@vertex
fn main(in : Input) -> Output {
  return Output(in.position);
}
