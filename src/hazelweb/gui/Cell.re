module Vdom = Virtual_dom.Vdom;
module Dom = Js_of_ocaml.Dom;
module Dom_html = Js_of_ocaml.Dom_html;
module Js = Js_of_ocaml.Js;
module Sexp = Sexplib.Sexp;

open ViewUtil;

let view_of_cursor_inspector =
    (
      ~inject,
      ~font_metrics: FontMetrics.t,
      ~is_mac: bool,
      ~settings: Settings.t,
      type_editor_is_focused,
      assistant_editor,
      (steps, cursor): CursorPath.t,
      cursor_inspector: Settings.CursorInspector.t,
      cursor_info: CursorInfo.t,
      l: UHLayout.t,
      u_gen: MetaVarGen.t,
    ) => {
  let cursor =
    switch (cursor) {
    | OnText(_) => CursorPosition.OnText(0)
    | OnDelim(index, _) => CursorPosition.OnDelim(index, Before)
    | OnOp(_) => CursorPosition.OnOp(Before)
    };
  let m = UHMeasuredLayout.mk(l);
  let cursor_pos =
    UHMeasuredLayout.caret_position_of_path((steps, cursor), m)
    |> OptUtil.get(() => failwith("could not find caret"));
  let cursor_x = float_of_int(cursor_pos.col) *. font_metrics.col_width;
  let cursor_y = float_of_int(cursor_pos.row) *. font_metrics.row_height;
  CursorInspector.view(
    ~inject,
    ~font_metrics,
    ~is_mac,
    ~settings,
    type_editor_is_focused,
    assistant_editor,
    (cursor_x, cursor_y),
    cursor_inspector,
    cursor_info,
    u_gen,
  );
};

let code_view =
    (
      ~inject: ModelAction.t => Vdom.Event.t,
      ~font_metrics: FontMetrics.t,
      ~is_mac: bool,
      ~settings: Settings.t,
      program: Program.exp,
      focal_editor: Model.editor,
      assistant_editor: Program.typ,
    )
    : Vdom.Node.t => {
  TimeUtil.measure_time(
    "Cell.view_internal",
    settings.performance.measure && settings.performance.uhcode_view,
    () => {
      open Vdom;

      let type_editor_is_focused = focal_editor == Model.AssistantTypeEditor;
      let main_editor_is_focused = focal_editor == Model.MainProgram;

      let l = Program.Exp.get_layout(~settings, program);

      let (code_text, decorations) =
        UHCode.view(
          ~font_metrics,
          ~settings,
          ~is_focused=main_editor_is_focused,
          program,
        );
      let caret = {
        let caret_pos = Program.Exp.get_caret_position(~settings, program);
        main_editor_is_focused
          ? [UHDecoration.Caret.view(~font_metrics, caret_pos)] : [];
      };

      let cursor_inspector =
        // TODO(andrew): when below uncommented, assistant disappears on
        // interaction. check focus logic.
        if (/*main_editor_is_focused &&*/ settings.cursor_inspector.visible) {
          let path = Program.Exp.get_path(program);
          let ci = Program.Exp.get_cursor_info(program);
          [
            view_of_cursor_inspector(
              ~inject,
              ~font_metrics,
              ~is_mac,
              ~settings,
              type_editor_is_focused,
              assistant_editor,
              path,
              settings.cursor_inspector,
              ci,
              l,
              Program.EditState_Exp.get_ugen(program.edit_state),
            ),
          ];
        } else {
          [];
        };

      let key_handlers =
        main_editor_is_focused
          ? UHCode.key_handlers(
              ~settings,
              ~u_gen=Program.EditState_Exp.get_ugen(program.edit_state),
              ~inject,
              ~is_mac,
              ~cursor_info=Program.Exp.get_cursor_info(program),
              //TODO(andrew): clean up below
              ~assistant_active=
                settings.cursor_inspector.assistant
                && Assistant_common.valid_assistant_term(
                     Program.Exp.get_cursor_info(program).cursor_term,
                   ),
            )
          : [];

      let root_id = Model.editor_id(MainProgram);
      let click_handler = UHCode.click_handler(root_id, font_metrics, inject);

      let ci = Program.Exp.get_cursor_info(program);
      let ty =
        switch (Assistant_common.get_types_and_mode(ci.typed)) {
        | (None, _, _) => HTyp.Hole
        | (Some(ty), _, _) => ty
        };

      Node.div(
        [
          Attr.id(root_id),
          Attr.classes(["code", "presentation"]),
          // need to use mousedown instead of click to fire
          // (and move caret) before cell focus event handler
          Attr.on_mousedown(evt => {
            print_endline("setting cursor inspector invisible");
            Event.Many([
              click_handler(evt),
              inject(
                ModelAction.UpdateSettings(
                  CursorInspector(Set_visible(false)),
                ),
              ),
            ]);
          }),
          Attr.on_contextmenu(evt => {
            // TODO(andrew): make this sane
            Event.Many([
              Event.Prevent_default,
              //Event.Stop_propagation,
              click_handler(evt),
              //TODO(andrew): put this somewhere more reasonable?
              inject(SetAssistantTypeEditor(UHTyp.contract(ty))),
              inject(
                ModelAction.UpdateSettings(
                  CursorInspector(Set_visible(true)),
                ),
              ),
              inject(
                ModelAction.UpdateSettings(
                  CursorInspector(Reset_assistant_selection),
                ),
              ),
              inject(
                ModelAction.UpdateSettings(
                  CursorInspector(Set_assistant(true)),
                ),
              ),
            ])
          }),
          // necessary to make cell focusable
          Attr.create("tabindex", "0"),
          Attr.on_focus(_ => {
            print_endline("CELL taking focus");
            inject(ModelAction.FocusCell(Model.MainProgram));
          }),
          Attr.on_blur(_ => inject(ModelAction.BlurCell)),
          ...key_handlers,
        ],
        caret
        @ cursor_inspector
        @ [Node.span([Attr.classes(["code"])], code_text), ...decorations],
      );
    },
  );
};

let view = (~inject, model: Model.t) => {
  let settings = model.settings;
  let performance = settings.performance;
  TimeUtil.measure_time(
    "Cell.view",
    performance.measure && performance.cell_view,
    () => {
      open Vdom;
      let program = Model.get_program(model);
      Node.div(
        [Attr.id(cell_id)],
        [
          /* font-specimen used to gather font metrics for caret positioning and other things */
          Node.div([Attr.id("font-specimen")], [Node.text("X")]),
          Node.div(
            [Attr.id("code-container")],
            [
              code_view(
                ~inject,
                ~font_metrics=model.font_metrics,
                ~is_mac=model.is_mac,
                ~settings,
                program,
                model.focal_editor,
                model.assistant_editor,
              ),
            ],
          ),
        ],
      );
    },
  );
};
