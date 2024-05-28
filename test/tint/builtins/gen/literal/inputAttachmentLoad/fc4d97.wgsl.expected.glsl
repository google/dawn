SKIP: FAILED


enable chromium_internal_input_attachments;

@input_attachment_index(3) @group(1) @binding(0) var arg_0 : input_attachment<u32>;

fn inputAttachmentLoad_fc4d97() {
  var res : vec4<u32> = inputAttachmentLoad(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@fragment
fn fragment_main() {
  inputAttachmentLoad_fc4d97();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/inputAttachmentLoad/fc4d97.wgsl:44:44 error: unresolved value 'arg_0'
  var res: vec4<u32> = inputAttachmentLoad(arg_0);
                                           ^^^^^

