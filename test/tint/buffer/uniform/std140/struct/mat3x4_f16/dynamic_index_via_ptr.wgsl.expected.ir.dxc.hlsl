SKIP: FAILED


enable f16;

struct Inner {
  @size(64)
  m : mat3x4<f16>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat3x4<f16> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec4<f16> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f16 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:60:5 note: %23 declared here
    %23:u32 = mul %48, 2u
    ^^^^^^^

:43:24 error: binary: %23 is not in scope
    %29:u32 = add %28, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:60:5 note: %23 declared here
    %23:u32 = mul %48, 2u
    ^^^^^^^

:60:5 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

    %23:u32 = mul %48, 2u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(8) {
  m:mat3x4<f16> @offset(0)
}

Outer = struct @align(8) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 8u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:u32 = add %10, %13
    %28:u32 = add %27, %16
    %29:u32 = add %28, %23
    %30:array<Inner, 4> = call %31, %29
    %l_a_i_a:array<Inner, 4> = let %30
    %33:u32 = add %10, %13
    %34:Inner = call %35, %33
    %l_a_i_a_i:Inner = let %34
    %37:u32 = add %10, %13
    %38:mat3x4<f16> = call %39, %37
    %l_a_i_a_i_m:mat3x4<f16> = let %38
    %41:u32 = add %10, %13
    %42:u32 = add %41, %16
    %43:u32 = div %42, 16u
    %44:ptr<uniform, vec4<u32>, read> = access %a, %43
    %45:vec4<u32> = load %44
    %46:vec4<f16> = bitcast %45
    %l_a_i_a_i_m_i:vec4<f16> = let %46
    %48:i32 = call %i
    %23:u32 = mul %48, 2u
    %49:u32 = add %10, %13
    %50:u32 = add %49, %16
    %51:u32 = add %50, %23
    %52:u32 = div %51, 16u
    %53:ptr<uniform, vec4<u32>, read> = access %a, %52
    %54:u32 = mod %51, 16u
    %55:u32 = div %54, 4u
    %56:u32 = load_vector_element %53, %55
    %57:u32 = mod %51, 4u
    %58:bool = eq %57, 0u
    %59:u32 = hlsl.ternary 16u, 0u, %58
    %60:u32 = shr %56, %59
    %61:f32 = hlsl.f16tof32 %60
    %62:f16 = convert %61
    %l_a_i_a_i_m_i_i:f16 = let %62
    ret
  }
}
%35 = func(%start_byte_offset:u32):Inner {
  $B4: {
    %65:mat3x4<f16> = call %39, %start_byte_offset
    %66:Inner = construct %65
    ret %66
  }
}
%39 = func(%start_byte_offset_1:u32):mat3x4<f16> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %68:u32 = div %start_byte_offset_1, 16u
    %69:ptr<uniform, vec4<u32>, read> = access %a, %68
    %70:vec4<u32> = load %69
    %71:vec4<f16> = bitcast %70
    %72:u32 = add 8u, %start_byte_offset_1
    %73:u32 = div %72, 16u
    %74:ptr<uniform, vec4<u32>, read> = access %a, %73
    %75:vec4<u32> = load %74
    %76:vec4<f16> = bitcast %75
    %77:u32 = add 16u, %start_byte_offset_1
    %78:u32 = div %77, 16u
    %79:ptr<uniform, vec4<u32>, read> = access %a, %78
    %80:vec4<u32> = load %79
    %81:vec4<f16> = bitcast %80
    %82:mat3x4<f16> = construct %71, %76, %81
    ret %82
  }
}
%31 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat3x4<f16>(vec4<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %86:bool = gte %idx, 4u
        if %86 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %87:u32 = mul %idx, 64u
        %88:u32 = add %start_byte_offset_2, %87
        %89:ptr<function, Inner, read_write> = access %a_1, %idx
        %90:Inner = call %35, %88
        store %89, %90
        continue  # -> $B9
      }
      $B9: {  # continuing
        %91:u32 = add %idx, 1u
        next_iteration %91  # -> $B8
      }
    }
    %92:array<Inner, 4> = load %a_1
    ret %92
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %94:array<Inner, 4> = call %31, %start_byte_offset_3
    %95:Outer = construct %94
    ret %95
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat3x4<f16>(vec4<f16>(0.0h))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %99:bool = gte %idx_1, 4u
        if %99 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %100:u32 = mul %idx_1, 256u
        %101:u32 = add %start_byte_offset_4, %100
        %102:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %103:Outer = call %25, %101
        store %102, %103
        continue  # -> $B15
      }
      $B15: {  # continuing
        %104:u32 = add %idx_1, 1u
        next_iteration %104  # -> $B14
      }
    }
    %105:array<Outer, 4> = load %a_2
    ret %105
  }
}

