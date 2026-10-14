SKIP: FAILED

%fs = @fragment func():void {
  $B1: {
    %2:bool = hasResource<texture_2d<i32>> 4u
    %t:bool = let %2
    ret
  }
}
Failed to generate: resource tables not supported by the HLSL backend for compiling with FXC

tint executable returned error: exit status 1
