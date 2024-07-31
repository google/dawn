SKIP: FAILED


var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<u32>;

fn atomicSub_0d26c2() {
  var res = 0u;
  let x_10 = atomicSub(&(arg_0), 1u);
  res = x_10;
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0u);
  workgroupBarrier();
  atomicSub_0d26c2();
  return;
}

fn compute_main_1() {
  let x_30 = local_invocation_index_1;
  compute_main_inner(x_30);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

Failed to generate: :14:5 error: unary: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'

    %6:u32 = negation 1u
    ^^^^^^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
compute_main_inputs = struct @align(4) {
  local_invocation_index_1_param:u32 @offset(0), @builtin(local_invocation_index)
}

$B1: {  # root
  %local_invocation_index_1:ptr<private, u32, read_write> = var
  %arg_0:ptr<workgroup, atomic<u32>, read_write> = var
}

%atomicSub_0d26c2 = func():void {
  $B2: {
    %res:ptr<function, u32, read_write> = var, 0u
    %5:ptr<function, u32, read_write> = var, 0u
    %6:u32 = negation 1u
    %7:void = hlsl.InterlockedAdd %arg_0, %6, %5
    %8:u32 = load %5
    %x_10:u32 = let %8
    store %res, %x_10
    ret
  }
}
%compute_main_inner = func(%local_invocation_index_2:u32):void {
  $B3: {
    %12:ptr<function, u32, read_write> = var, 0u
    %13:void = hlsl.InterlockedExchange %arg_0, 0u, %12
    %14:void = workgroupBarrier
    %15:void = call %atomicSub_0d26c2
    ret
  }
}
%compute_main_1 = func():void {
  $B4: {
    %17:u32 = load %local_invocation_index_1
    %x_30:u32 = let %17
    %19:void = call %compute_main_inner, %x_30
    ret
  }
}
%compute_main_inner_1 = func(%local_invocation_index_1_param:u32):void {  # %compute_main_inner_1: 'compute_main_inner'
  $B5: {
    %22:bool = eq %local_invocation_index_1_param, 0u
    if %22 [t: $B6] {  # if_1
      $B6: {  # true
        %23:ptr<function, u32, read_write> = var, 0u
        %24:void = hlsl.InterlockedExchange %arg_0, 0u, %23
        exit_if  # if_1
      }
    }
    %25:void = workgroupBarrier
    store %local_invocation_index_1, %local_invocation_index_1_param
    %26:void = call %compute_main_1
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func(%inputs:compute_main_inputs):void {
  $B7: {
    %29:u32 = access %inputs, 0u
    %30:void = call %compute_main_inner_1, %29
    ret
  }
}

