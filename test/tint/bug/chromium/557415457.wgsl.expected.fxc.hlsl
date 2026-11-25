SKIP: INVALID

$B1: {  # root
  %tex:ptr<handle, texture_storage_2d<rgba8unorm, read_write>, read> = var @binding_point(0, 0)
}

%fs = @fragment func():void {
  $B2: {
    %3:bool = hasResource<texture_2d<i32>> 4u
    %tex_1:bool = let %3  # %tex_1: 'tex'
    ret
  }
}
Failed to generate: resource tables not supported by the HLSL backend for compiling with FXC

tint executable returned error: exit status 1
