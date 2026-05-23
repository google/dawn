SKIP: FAILED

$B1: {  # root
  %s:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
}

%fs = @fragment func():vec4<f32> [@location(0)] {
  $B2: {
    %3:texture_2d<f32> = getResource<texture_2d<f32>> 0i
    %4:sampler = load %s
    %5:vec4<f32> = textureSample %3, %4, vec2<f32>(0.0f)
    ret %5
  }
}
Failed to generate: resource tables not supported by the HLSL backend for compiling with FXC

tint executable returned error: exit status 1
