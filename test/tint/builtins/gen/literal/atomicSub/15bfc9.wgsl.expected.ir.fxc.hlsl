SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

struct SB_RW {
  arg_0 : atomic<u32>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn atomicSub_15bfc9() -> u32 {
  var res : u32 = atomicSub(&(sb_rw.arg_0), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = atomicSub_15bfc9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicSub_15bfc9();
}

Failed to generate: :13:5 error: unary: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'

    %5:u32 = negation 1u
    ^^^^^^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
SB_RW = struct @align(4) {
  arg_0:atomic<u32> @offset(0)
}

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 0)
  %sb_rw:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 1)
}

%atomicSub_15bfc9 = func():u32 {
  $B2: {
    %4:ptr<function, u32, read_write> = var, 0u
    %5:u32 = negation 1u
    %6:u32 = convert 0u
    %7:void = %sb_rw.InterlockedAdd %6, %5, %4
    %8:u32 = load %4
    %res:ptr<function, u32, read_write> = var, %8
    %10:u32 = load %res
    ret %10
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %12:u32 = call %atomicSub_15bfc9
    %13:u32 = bitcast %12
    %14:void = %prevent_dce.Store 0u, %13
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %16:u32 = call %atomicSub_15bfc9
    %17:u32 = bitcast %16
    %18:void = %prevent_dce.Store 0u, %17
    ret
  }
}


tint executable returned error: exit status 1
