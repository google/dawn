SKIP: INVALID


enable chromium_internal_input_attachments;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@input_attachment_index(3) @group(1) @binding(0) var arg_0 : input_attachment<f32>;

fn inputAttachmentLoad_c38b2f() -> vec4<f32> {
  var res : vec4<f32> = inputAttachmentLoad(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = inputAttachmentLoad_c38b2f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/inputAttachmentLoad/c38b2f.wgsl:47:44 error: unresolved value 'arg_0'
  var res: vec4<f32> = inputAttachmentLoad(arg_0);
                                           ^^^^^


tint executable returned error: exit status 1
