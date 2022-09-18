open Virtual_dom.Vdom;
open Node;
open Util.Web;
open Haz3lcore;

/* TODO copied from Page */
let toggle = (label, active, action) =>
  div(
    ~attr=
      Attr.many([
        clss(["toggle-switch"] @ (active ? ["active"] : [])),
        Attr.on_click(action),
      ]),
    [div(~attr=clss(["toggle-knob"]), [text(label)])],
  );

let feedback_view = (message, up_active, up_action, down_active, down_action) => {
  div(
    ~attr=clss(["feedback"]),
    [
      div(~attr=clss(["message"]), [text(message)]),
      div(
        ~attr=
          Attr.many([
            clss(["option"] @ (up_active ? ["active"] : [])),
            Attr.on_click(up_action),
          ]),
        [text("👍")],
      ),
      div(
        ~attr=
          Attr.many([
            clss(["option"] @ (down_active ? ["active"] : [])),
            Attr.on_click(down_action),
          ]),
        [text("👎")],
      ),
    ],
  );
};

let explanation_feedback_view =
    (~inject, id, explanation: LangDocMessages.explanation) => {
  let (up_active, down_active) =
    switch (explanation.feedback) {
    | ThumbsUp => (true, false)
    | ThumbsDown => (false, true)
    | Unselected => (false, false)
    };
  feedback_view(
    "This explanation is helpful",
    up_active,
    _ =>
      inject(
        Update.UpdateLangDocMessages(
          ToggleExplanationFeedback(id, ThumbsUp),
        ),
      ),
    down_active,
    _ =>
      inject(
        Update.UpdateLangDocMessages(
          ToggleExplanationFeedback(id, ThumbsDown),
        ),
      ),
  );
};

let example_feedback_view = (~inject, id, example: LangDocMessages.example) => {
  let (up_active, down_active) =
    switch (example.feedback) {
    | ThumbsUp => (true, false)
    | ThumbsDown => (false, true)
    | Unselected => (false, false)
    };
  feedback_view(
    "This example is helpful",
    up_active,
    _ =>
      inject(
        Update.UpdateLangDocMessages(
          ToggleExampleFeedback(id, example.sub_id, ThumbsUp),
        ),
      ),
    down_active,
    _ =>
      inject(
        Update.UpdateLangDocMessages(
          ToggleExampleFeedback(id, example.sub_id, ThumbsDown),
        ),
      ),
  );
};

/* TODO - Hannah - this is used (or something pretty similar) other places and should probably be refactored to somewhere
   centeralized like AssistantView_common - or maybe the different uses are different enough... */
let code_node = text => Node.span(~attr=clss(["code"]), [Node.text(text)]);

let highlight =
    (msg: list(Node.t), id: Haz3lcore.Id.t, mapping: ColorSteps.t)
    : (Node.t, ColorSteps.t) => {
  let (c, mapping) = ColorSteps.get_color(id, mapping);
  /*print_endline(
      "Color chosen at highlight: ("
      ++ Sexp.to_string(CursorPath.sexp_of_steps(steps))
      ++ ", "
      ++ c
      ++ ")",
    );*/
  (Node.span(~attr=clss(["highlight-" ++ c]), msg), mapping);
};

let _max_elems = 7;
let int_to_word_number = (n: int): string => {
  switch (n) {
  | 1 => "first"
  | 2 => "second"
  | 3 => "third"
  | 4 => "fourth"
  | 5 => "fifth"
  | 6 => "sixth"
  | 7 => "seventh"
  | _ => ""
  };
};
let comma_separated_list = (items: list(string)): string => {
  /*let _ = List.map(item => print_endline(item), items);*/
  let length = List.length(items);
  let items =
    List.mapi(
      (index, item) => {
        let separator =
          if (index == length - 1) {
            length > 2 ? ", and" : " and";
          } else if (index == 0) {
            "";
          } else {
            ",";
          };
        separator ++ " " ++ item;
      },
      items,
    );
  List.fold_left((acc, item) => acc ++ item, "", items);
};

let print_markdown = doc => {
  print_endline("-----------------BEGIN PRINTING------------------");
  let rec print_markdown' = doc => {
    let _ =
      List.mapi(
        (index, element) => {
          print_endline(string_of_int(index));
          switch (element) {
          | Omd.Paragraph(d) =>
            print_endline("------Paragraph---------");
            print_markdown'(d);
          | Ul(_items) => print_endline("Ul")
          | Ulp(_items) => print_endline("Ul  PPPPPP")
          | Text(_) => print_endline("Text")
          | Url(_, d, _) =>
            print_endline("URL");
            print_markdown'(d);
          | Code(_) => print_endline("Code")
          | _ => print_endline("Something else")
          };
        },
        doc,
      );
    ();
  };
  print_markdown'(doc);
  print_endline("---------------------END PRINTING-----------------");
};

/*
 Markdown like thing:
 highlighty thing : [thing to highlight](id)
 bulleted list: - list item
                - list item
 code: `code`
 italics: *word*
 */
let mk_explanation =
    (~inject, id, explanation, text: string, show_highlight: bool)
    : (Node.t, ColorSteps.t) => {
  let omd = Omd.of_string(text);
  //print_markdown(omd);
  let rec translate =
          (doc: Omd.t, mapping: ColorSteps.t): (list(Node.t), ColorSteps.t) =>
    List.fold_left(
      ((msg, mapping), elem) => {
        switch (elem) {
        | Omd.Paragraph(d) => translate(d, mapping)
        | Text(t) => (List.append(msg, [Node.text(t)]), mapping)
        | Ul(items) =>
          //print_endline("IN THE LIST THINGY");
          let (bullets, mapping) =
            List.fold_left(
              ((nodes, mapping), d) => {
                let (n, mapping) = translate(d, mapping);
                (List.append(nodes, [Node.li(n)]), mapping);
              },
              ([], mapping),
              items,
            );
          (List.append(msg, [Node.ul(bullets)]), mapping); /* TODO Hannah - Should this be an ordered list instead of an unordered list? */
        | Code(_name, t) => (List.append(msg, [code_node(t)]), mapping)
        | Url(id, d, _title) =>
          let (d, mapping) = translate(d, mapping);
          let (inner_msg, mapping) =
            if (show_highlight) {
              let id = int_of_string(id);
              highlight(d, id, mapping);
            } else {
              (Node.span(d), mapping);
            };
          (List.append(msg, [inner_msg]), mapping);
        | Emph(d) =>
          let (d, mapping) = translate(d, mapping);
          (
            List.append(
              msg,
              [
                Node.span(
                  ~attr=
                    Attr.style(
                      Css_gen.create(~field="font-style", ~value="italic"),
                    ),
                  d,
                ),
              ],
            ),
            mapping,
          );
        | _ =>
          print_endline("OTHER");
          (msg, mapping);
        }
      },
      ([], mapping),
      doc,
    );
  let (msg, color_map) = translate(omd, ColorSteps.empty);
  (
    div([
      div(~attr=clss(["explanation-contents"]), msg),
      explanation_feedback_view(~inject, id, explanation),
    ]),
    color_map,
  );
};

let deco =
    (
      ~doc: LangDocMessages.t,
      ~settings,
      ~colorings,
      ~expandable: option(Id.t),
      ~unselected,
      ~map,
      ~inject,
      ~font_metrics,
      ~options,
      ~group_id,
      ~form_id,
    ) => {
  module Deco =
    Deco.Deco({
      let font_metrics = font_metrics;
      let map = map;
      let show_backpack_targets = false;
      let (term, terms) = MakeTerm.go(unselected);
      let info_map = Statics.mk_map(term);
      let term_ranges = TermRanges.mk(unselected);
      let tiles = TileMap.mk(unselected);
    });

  let term_lang_doc =
    switch (expandable) {
    | None => []
    | Some(expandable) =>
      print_endline("EXPANDABLE: " ++ string_of_int(expandable));
      [
        Deco.term_decoration(
          ~id=expandable,
          ((origin, path)) => {
            let specificity_pos =
              Printf.sprintf(
                "position: absolute; top: %fpx;",
                font_metrics.row_height,
              );

            let specificity_style =
              Attr.create(
                "style",
                specificity_pos
                ++ (doc.specificity_open ? "transform: scaleY(1);" : ""),
              );

            let specificity_menu =
              Node.div(
                ~attr=
                  Attr.many([
                    clss(["specificity-options-menu", "expandable"]),
                    specificity_style,
                  ]),
                List.mapi(
                  (index, (id, segment)) => {
                    print_endline("Drawing specificity menu items");
                    let map = Measured.of_segment(segment);
                    let code_view =
                      Code.simple_view(~unselected=segment, ~map, ~settings);
                    id == form_id
                      ? Node.div(
                          ~attr=
                            Attr.many([
                              clss(["selected"]),
                              Attr.on_click(_ =>
                                inject(
                                  Update.UpdateLangDocMessages(
                                    LangDocMessages.UpdateGroupSelection(
                                      group_id,
                                      index,
                                    ),
                                  ),
                                )
                              ),
                            ]),
                          [code_view],
                        )
                      : Node.div(
                          ~attr=
                            Attr.on_click(_ =>
                              inject(
                                Update.UpdateLangDocMessages(
                                  LangDocMessages.UpdateGroupSelection(
                                    group_id,
                                    index,
                                  ),
                                ),
                              )
                            ),
                          [code_view],
                        );
                  },
                  options,
                ),
              );
            print_endline("TRYING TO DRAW EXPANDABLE");
            let expandable_deco =
              DecUtil.code_svg(
                ~font_metrics,
                ~origin,
                ~base_cls=["expandable"],
                ~abs_pos=false,
                path,
              );
            print_endline("FINISHED DRAWING EXPANDABLE");
            Node.div(
              ~attr=
                Attr.many([
                  clss(["expandable-target"]),
                  DecUtil.abs_position(~font_metrics, origin),
                  Attr.on_click(_ => {
                    inject(
                      Update.UpdateLangDocMessages(
                        LangDocMessages.SpecificityOpen(
                          !doc.specificity_open,
                        ),
                      ),
                    )
                  }),
                ]),
              [expandable_deco, specificity_menu],
            );
          },
        ),
      ];
    };

  let color_highlight =
    if (doc.highlight) {
      List.map(
        ((id, color)) =>
          Deco.term_highlight(~clss=["highlight-code-" ++ color], id),
        colorings,
      );
    } else {
      [];
    };
  color_highlight @ term_lang_doc;
};

let syntactic_form_view =
    (
      ~doc,
      ~colorings,
      ~expandable,
      ~inject,
      ~font_metrics,
      ~unselected,
      ~settings,
      ~id,
      ~options,
      ~group_id,
      ~form_id,
    ) => {
  let map = Measured.of_segment(unselected);
  let code_view = Code.simple_view(~unselected, ~map, ~settings);
  let deco_view =
    deco(
      ~doc,
      ~settings,
      ~colorings,
      ~expandable,
      ~unselected,
      ~map,
      ~inject,
      ~font_metrics,
      ~options,
      ~group_id,
      ~form_id,
    );
  div(
    ~attr=Attr.many([Attr.id(id), Attr.class_("code-container")]),
    [code_view] @ deco_view,
  );
};

let example_view =
    (
      ~inject,
      ~font_metrics,
      ~settings,
      ~id,
      ~examples: list(LangDocMessages.example),
    ) => {
  div(
    ~attr=Attr.id("examples"),
    List.length(examples) == 0
      ? [text("No examples available")]
      : List.map(
          ({term, message, _} as example: LangDocMessages.example) => {
            let map_code = Measured.of_segment(term);
            let code_view =
              Code.simple_view(~unselected=term, ~map=map_code, ~settings);
            let (uhexp, _) = MakeTerm.go(term);
            let info_map = Statics.mk_map(uhexp);
            let result_view =
              switch (Interface.evaluation_result(info_map, uhexp)) {
              | None => []
              | Some(dhexp) => [SimpleMode.res_view(~font_metrics, dhexp)]
              };
            let code_container = view =>
              div(~attr=clss(["code-container"]), view);
            div(
              ~attr=clss(["example"]),
              [
                code_container([code_view]),
                div(
                  ~attr=clss(["ex-result"]),
                  [text("Result: "), code_container(result_view)],
                ),
                div(
                  ~attr=clss(["explanation"]),
                  [text("Explanation: "), text(message)],
                ),
                example_feedback_view(~inject, id, example),
              ],
            );
          },
          examples,
        ),
  );
};

let rec bypass_parens_and_annot_pat = pat => {
  switch (pat) {
  | TermBase.UPat.Parens(p)
  | TypeAnn(p, _) => bypass_parens_and_annot_pat(p.term)
  | _ => pat
  };
};

let rec bypass_parens_exp = exp => {
  switch (exp) {
  | TermBase.UExp.Parens(e) => bypass_parens_exp(e.term)
  | _ => exp
  };
};

let get_doc =
    (
      ~inject,
      ~font_metrics,
      ~settings: Model.settings,
      ~docs: LangDocMessages.t,
      info: option(Statics.t),
    ) => {
  let default = (
    text("No syntactic form available"),
    (text("No explanation available"), ColorSteps.empty),
    text("No examples available"),
  );
  let get_message =
      (
        doc: LangDocMessages.form,
        options,
        group_id,
        explanation_msg,
        colorings,
      ) => {
    print_endline("Making the explanation");
    let (explanation, color_map) =
      mk_explanation(
        ~inject,
        doc.id,
        doc.explanation,
        explanation_msg,
        docs.highlight,
      );
    print_endline("Making the syntactic form");
    let syntactic_form_view =
      syntactic_form_view(
        ~doc=docs,
        ~colorings=
          List.map(
            ((syntactic_form_id: int, code_id: int)) => {
              let (color, _) = ColorSteps.get_color(code_id, color_map);
              (syntactic_form_id, color);
            },
            colorings,
          ),
        ~expandable=doc.expandable_id,
        ~inject,
        ~font_metrics,
        ~unselected=doc.syntactic_form,
        ~settings,
        ~id="syntactic-form-code",
        ~options,
        ~group_id,
        ~form_id=doc.id,
      );
    print_endline("Making the example");
    let example_view =
      example_view(
        ~inject,
        ~font_metrics,
        ~settings,
        ~id=doc.id,
        ~examples=doc.examples,
      );
    (syntactic_form_view, (explanation, color_map), example_view);
  };

  switch (info) {
  | Some(InfoExp({term, _})) =>
    let rec get_message_exp = term =>
      switch (term) {
      | TermBase.UExp.Invalid(_) => default
      | EmptyHole =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.empty_hole_exp_group,
            docs,
          );
        get_message(
          doc,
          options,
          LangDocMessages.empty_hole_exp_group,
          doc.explanation.message,
          [],
        );
      | MultiHole(_children) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.multi_hole_exp_group,
            docs,
          );
        get_message(
          doc,
          options,
          LangDocMessages.multi_hole_exp_group,
          doc.explanation.message,
          [],
        );
      | Triv =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.triv_exp_group,
            docs,
          );
        get_message(
          doc,
          options,
          LangDocMessages.triv_exp_group,
          doc.explanation.message,
          [],
        );
      | Bool(_bool_lit) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.bool_exp_group,
            docs,
          );
        get_message(
          doc,
          options,
          LangDocMessages.bool_exp_group,
          doc.explanation.message,
          [],
        );
      | Int(_int_lit) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.int_exp_group,
            docs,
          );
        get_message(
          doc,
          options,
          LangDocMessages.int_exp_group,
          doc.explanation.message,
          [],
        );
      | Float(_float_lit) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.float_exp_group,
            docs,
          );
        get_message(
          doc,
          options,
          LangDocMessages.float_exp_group,
          doc.explanation.message,
          [],
        );
      | ListLit(terms) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.list_exp_group,
            docs,
          );
        get_message(
          doc,
          options,
          LangDocMessages.list_exp_group,
          Printf.sprintf(
            Scanf.format_from_string(doc.explanation.message, "%i"),
            List.length(terms),
          ),
          [],
        );
      | Fun(pat, body) =>
        //print_endline("Getting the function group");

        let basic = (doc: LangDocMessages.form, group_id, options) => {
          //print_endline(
          //  "BODY: " ++ Sexplib.Sexp.to_string(TermBase.UExp.sexp_of_t(body)),
          //); // The body is a seq with a bunch of extra stuff in it...
          // and the top id isn't anywhere in the segment
          //print_endline("Here2");
          //let _ = List.map(i => print_endline(string_of_int(i)), body.ids);
          //let _ = List.map(i => print_endline(string_of_int(i)), pat.ids);
          let pat_id = List.nth(pat.ids, 0);
          let body_id = List.nth(body.ids, 0);
          //print_endline(
          //  Sexplib.Sexp.to_string(Segment.sexp_of_t(doc.syntactic_form)),
          //);
          let pat_coloring_ids =
            switch (List.nth(doc.syntactic_form, 0)) {
            | Tile(tile) => [
                (
                  Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                  pat_id,
                ),
              ]
            | _ => []
            };
          //print_endline("GOT THE PAT COLOR ID");
          get_message(
            doc,
            options,
            group_id,
            Printf.sprintf(
              Scanf.format_from_string(doc.explanation.message, "%i%i"),
              pat_id,
              body_id,
            ),
            pat_coloring_ids
            @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
          );
        };
        switch (bypass_parens_and_annot_pat(pat.term)) {
        | EmptyHole =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.function_empty_hole_group,
              docs,
            );
          if (LangDocMessages.function_empty_hole_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.function_empty_hole_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
                pat_id,
                pat_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.function_empty_hole_group, options);
          };
        | MultiHole(_) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.function_multi_hole_group,
              docs,
            );
          if (LangDocMessages.function_multi_hole_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.function_multi_hole_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
                pat_id,
                pat_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.function_multi_hole_group, options);
          };
        | Wild =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.function_wild_group,
              docs,
            );
          if (LangDocMessages.function_wild_exp.id == doc.id) {
            let body_id = List.nth(body.ids, 0);
            get_message(
              doc,
              options,
              LangDocMessages.function_wild_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i"),
                body_id,
              ),
              [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.function_wild_group, options);
          };
        | Int(i) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.function_int_group,
              docs,
            );
          if (LangDocMessages.function_intlit_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.function_int_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i%i"),
                pat_id,
                i,
                pat_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.function_int_group, options);
          };
        | Float(f) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.function_float_group,
              docs,
            );
          if (LangDocMessages.function_floatlit_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.function_float_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%f%i%i"),
                pat_id,
                f,
                pat_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.function_float_group, options);
          };
        | Bool(b) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.function_bool_group,
              docs,
            );
          if (LangDocMessages.function_boollit_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.function_bool_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%b%i%i"),
                pat_id,
                b,
                pat_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.function_bool_group, options);
          };
        | Triv =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.function_triv_group,
              docs,
            );
          if (LangDocMessages.function_triv_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.function_triv_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
                pat_id,
                pat_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.function_triv_group, options);
          };
        | ListLit(elements) =>
          if (List.length(elements) == 0) {
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.function_listnil_group,
                docs,
              );
            if (LangDocMessages.function_listnil_exp.id == doc.id) {
              let pat_id = List.nth(pat.ids, 0);
              let body_id = List.nth(body.ids, 0);
              let pat_coloring_ids =
                switch (List.nth(doc.syntactic_form, 0)) {
                | Tile(tile) => [
                    (
                      Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                      pat_id,
                    ),
                  ]
                | _ => []
                };
              get_message(
                doc,
                options,
                LangDocMessages.function_listnil_group,
                Printf.sprintf(
                  Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
                  pat_id,
                  pat_id,
                  body_id,
                ),
                pat_coloring_ids
                @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
              );
            } else {
              basic(doc, LangDocMessages.function_listnil_group, options);
            };
          } else {
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.function_listlit_group,
                docs,
              );
            if (LangDocMessages.function_listlit_exp.id == doc.id) {
              let pat_id = List.nth(pat.ids, 0);
              let body_id = List.nth(body.ids, 0);
              let pat_coloring_ids =
                switch (List.nth(doc.syntactic_form, 0)) {
                | Tile(tile) => [
                    (
                      Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                      pat_id,
                    ),
                  ]
                | _ => []
                };
              get_message(
                doc,
                options,
                LangDocMessages.function_listlit_group,
                Printf.sprintf(
                  Scanf.format_from_string(
                    doc.explanation.message,
                    "%i%i%i%i",
                  ),
                  pat_id,
                  List.length(elements),
                  pat_id,
                  body_id,
                ),
                pat_coloring_ids
                @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
              );
            } else {
              basic(doc, LangDocMessages.function_listlit_group, options);
            };
          }
        | Cons(hd, tl) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.function_cons_group,
              docs,
            );
          if (LangDocMessages.function_cons_exp.id == doc.id) {
            let hd_id = List.nth(hd.ids, 0);
            let tl_id = List.nth(tl.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) =>
                let pat = List.nth(tile.children, 0);
                [
                  (Piece.id(List.nth(pat, 0)), hd_id),
                  (Piece.id(List.nth(pat, 2)), tl_id),
                ];
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.function_cons_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
                hd_id,
                tl_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.function_cons_group, options);
          };
        | Var(var) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.function_var_group,
              docs,
            );
          if (LangDocMessages.function_var_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.function_var_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%s%i"),
                pat_id,
                var,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.function_var_group, options);
          };
        | Tuple(elements) =>
          let pat_id = List.nth(pat.ids, 0);
          let body_id = List.nth(body.ids, 0);
          let basic_tuple = (doc: LangDocMessages.form, group_id, options) => {
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 1)),
                    pat_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              group_id,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i%i"),
                pat_id,
                List.length(elements),
                pat_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          };

          switch (List.length(elements)) {
          | 2 =>
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.function_tuple_2_group,
                docs,
              );
            if (LangDocMessages.function_tuple2_exp.id == doc.id) {
              let pat1_id = List.nth(List.nth(elements, 0).ids, 0);
              let pat2_id = List.nth(List.nth(elements, 1).ids, 0);
              let pat_coloring_ids =
                switch (List.nth(doc.syntactic_form, 0)) {
                | Tile(tile) =>
                  let pat = List.nth(tile.children, 0);
                  [
                    (Piece.id(List.nth(pat, 0)), pat1_id),
                    (Piece.id(List.nth(pat, 2)), pat2_id),
                  ];
                | _ => []
                };
              get_message(
                doc,
                options,
                LangDocMessages.function_tuple_2_group,
                Printf.sprintf(
                  Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
                  pat1_id,
                  pat2_id,
                  body_id,
                ),
                pat_coloring_ids
                @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
              );
            } else if (LangDocMessages.function_tuple_exp.id == doc.id) {
              basic_tuple(
                doc,
                LangDocMessages.function_tuple_2_group,
                options,
              );
            } else {
              basic(doc, LangDocMessages.function_tuple_2_group, options);
            };
          | 3 =>
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.function_tuple_3_group,
                docs,
              );
            if (LangDocMessages.function_tuple3_exp.id == doc.id) {
              let pat1_id = List.nth(List.nth(elements, 0).ids, 0);
              let pat2_id = List.nth(List.nth(elements, 1).ids, 0);
              let pat3_id = List.nth(List.nth(elements, 2).ids, 0);
              let pat_coloring_ids =
                switch (List.nth(doc.syntactic_form, 0)) {
                | Tile(tile) =>
                  let pat = List.nth(tile.children, 0);
                  [
                    (Piece.id(List.nth(pat, 0)), pat1_id),
                    (Piece.id(List.nth(pat, 2)), pat2_id),
                    (Piece.id(List.nth(pat, 4)), pat3_id),
                  ];
                | _ => []
                };
              get_message(
                doc,
                options,
                LangDocMessages.function_tuple_3_group,
                Printf.sprintf(
                  Scanf.format_from_string(
                    doc.explanation.message,
                    "%i%i%i%i",
                  ),
                  pat1_id,
                  pat2_id,
                  pat3_id,
                  body_id,
                ),
                pat_coloring_ids
                @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
              );
            } else if (LangDocMessages.function_tuple_exp.id == doc.id) {
              basic_tuple(
                doc,
                LangDocMessages.function_tuple_3_group,
                options,
              );
            } else {
              basic(doc, LangDocMessages.function_tuple_3_group, options);
            };
          | _ =>
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.function_tuple_group,
                docs,
              );
            if (LangDocMessages.function_tuple_exp.id == doc.id) {
              basic_tuple(doc, LangDocMessages.function_tuple_group, options);
            } else {
              basic(doc, LangDocMessages.function_tuple_group, options);
            };
          };
        | Invalid(_) => default // Shouldn't get hit
        | Parens(_) => default // Shouldn't get hit?
        | TypeAnn(_) => default // Shouldn't get hit?
        };
      | Tuple(terms) =>
        let basic = (doc, group_id, options) =>
          get_message(
            doc,
            options,
            group_id,
            Printf.sprintf(
              Scanf.format_from_string(doc.explanation.message, "%i"),
              List.length(terms),
            ),
            [],
          );
        switch (List.length(terms)) {
        | 2 =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.tuple_exp_2_group,
              docs,
            );
          if (LangDocMessages.tuple_exp_size2.id == doc.id) {
            let left_id = List.nth(List.nth(terms, 0).ids, 0);
            let right_id = List.nth(List.nth(terms, 1).ids, 0);
            get_message(
              doc,
              options,
              LangDocMessages.tuple_exp_2_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i"),
                left_id,
                right_id,
              ),
              [
                (Piece.id(List.nth(doc.syntactic_form, 0)), left_id),
                (Piece.id(List.nth(doc.syntactic_form, 2)), right_id),
              ],
            );
          } else {
            basic(doc, LangDocMessages.tuple_exp_2_group, options);
          };
        | 3 =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.tuple_exp_3_group,
              docs,
            );
          if (LangDocMessages.tuple_exp_size3.id == doc.id) {
            let first_id = List.nth(List.nth(terms, 0).ids, 0);
            let second_id = List.nth(List.nth(terms, 1).ids, 0);
            let third_id = List.nth(List.nth(terms, 2).ids, 0);
            get_message(
              doc,
              options,
              LangDocMessages.tuple_exp_3_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
                first_id,
                second_id,
                third_id,
              ),
              [
                (Piece.id(List.nth(doc.syntactic_form, 0)), first_id),
                (Piece.id(List.nth(doc.syntactic_form, 2)), second_id),
                (Piece.id(List.nth(doc.syntactic_form, 4)), third_id),
              ],
            );
          } else {
            basic(doc, LangDocMessages.tuple_exp_3_group, options);
          };
        | _ =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.tuple_exp_group,
              docs,
            );
          basic(doc, LangDocMessages.tuple_exp_group, options);
        };
      | Var(_var) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.var_exp_group,
            docs,
          );
        get_message(
          doc,
          options,
          LangDocMessages.var_exp_group,
          doc.explanation.message,
          [],
        );
      | Let(pat, def, body) =>
        let basic = (doc: LangDocMessages.form, group_id, options) => {
          //print_endline(
          //  "BODY: " ++ Sexplib.Sexp.to_string(TermBase.UExp.sexp_of_t(body)),
          //); // The body is a seq with a bunch of extra stuff in it...
          // and the top id isn't anywhere in the segment
          //print_endline("Here2");
          //let _ = List.map(i => print_endline(string_of_int(i)), body.ids);
          //let _ = List.map(i => print_endline(string_of_int(i)), pat.ids);
          let pat_id = List.nth(pat.ids, 0);
          let def_id = List.nth(def.ids, 0);
          let body_id = List.nth(body.ids, 0);
          //print_endline(
          //  Sexplib.Sexp.to_string(Segment.sexp_of_t(doc.syntactic_form)),
          //);
          let pat_coloring_ids =
            switch (List.nth(doc.syntactic_form, 0)) {
            | Tile(tile) => [
                (
                  Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                  pat_id,
                ),
                (
                  Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                  def_id,
                ),
              ]
            | _ => []
            };
          //print_endline("GOT THE PAT COLOR ID");
          get_message(
            doc,
            options,
            group_id,
            Printf.sprintf(
              Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
              pat_id,
              def_id,
              body_id,
            ),
            pat_coloring_ids
            @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
          );
        };
        switch (bypass_parens_and_annot_pat(pat.term)) {
        | EmptyHole =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.let_empty_hole_exp_group,
              docs,
            );
          if (LangDocMessages.let_empty_hole_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let def_id = List.nth(def.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.let_empty_hole_exp_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i%i"),
                pat_id,
                def_id,
                body_id,
                pat_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.let_empty_hole_exp_group, options);
          };
        | MultiHole(_) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.let_multi_hole_exp_group,
              docs,
            );
          if (LangDocMessages.let_multi_hole_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let def_id = List.nth(def.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.let_multi_hole_exp_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i%i"),
                pat_id,
                def_id,
                body_id,
                pat_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.let_multi_hole_exp_group, options);
          };
        | Wild =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.let_wild_exp_group,
              docs,
            );
          if (LangDocMessages.let_wild_exp.id == doc.id) {
            let def_id = List.nth(def.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.let_wild_exp_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
                def_id,
                def_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.let_wild_exp_group, options);
          };
        | Int(i) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.let_int_exp_group,
              docs,
            );
          if (LangDocMessages.let_int_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let def_id = List.nth(def.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.let_int_exp_group,
              Printf.sprintf(
                Scanf.format_from_string(
                  doc.explanation.message,
                  "%i%i%i%i%i",
                ),
                def_id,
                pat_id,
                i,
                def_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            /* TODO The coloring for the syntactic form is sometimes wrong here... */
            basic(
              doc,
              LangDocMessages.let_int_exp_group,
              options,
            );
          };
        | Float(f) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.let_float_exp_group,
              docs,
            );
          if (LangDocMessages.let_float_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let def_id = List.nth(def.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ]
              | _ => []
              };
            // TODO Make sure everywhere printing the float literal print it prettier
            get_message(
              doc,
              options,
              LangDocMessages.let_float_exp_group,
              Printf.sprintf(
                Scanf.format_from_string(
                  doc.explanation.message,
                  "%i%i%f%i%i",
                ),
                def_id,
                pat_id,
                f,
                def_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            /* TODO The coloring for the syntactic form is sometimes wrong here... */
            basic(
              doc,
              LangDocMessages.let_float_exp_group,
              options,
            );
          };
        | Bool(b) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.let_bool_exp_group,
              docs,
            );
          if (LangDocMessages.let_bool_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let def_id = List.nth(def.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ]
              | _ => []
              };
            // TODO Make sure everywhere printing the float literal print it prettier
            get_message(
              doc,
              options,
              LangDocMessages.let_bool_exp_group,
              Printf.sprintf(
                Scanf.format_from_string(
                  doc.explanation.message,
                  "%i%i%b%i%i",
                ),
                def_id,
                pat_id,
                b,
                def_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            /* TODO The coloring for the syntactic form is sometimes wrong here... */
            basic(
              doc,
              LangDocMessages.let_bool_exp_group,
              options,
            );
          };
        | Triv =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.let_triv_exp_group,
              docs,
            );
          if (LangDocMessages.let_triv_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let def_id = List.nth(def.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ]
              | _ => []
              };
            // TODO Make sure everywhere printing the float literal print it prettier
            get_message(
              doc,
              options,
              LangDocMessages.let_triv_exp_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%i%i"),
                def_id,
                pat_id,
                def_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            /* TODO The coloring for the syntactic form is sometimes wrong here and other places when switching syntactic specificities... */
            basic(
              doc,
              LangDocMessages.let_triv_exp_group,
              options,
            );
          };
        | ListLit(elements) =>
          if (List.length(elements) == 0) {
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.let_listnil_exp_group,
                docs,
              );
            if (LangDocMessages.let_listnil_exp.id == doc.id) {
              let pat_id = List.nth(pat.ids, 0);
              let def_id = List.nth(def.ids, 0);
              let body_id = List.nth(body.ids, 0);
              let pat_coloring_ids =
                switch (List.nth(doc.syntactic_form, 0)) {
                | Tile(tile) => [
                    (
                      Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                      pat_id,
                    ),
                    (
                      Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                      def_id,
                    ),
                  ]
                | _ => []
                };
              get_message(
                doc,
                options,
                LangDocMessages.let_listnil_exp_group,
                Printf.sprintf(
                  Scanf.format_from_string(
                    doc.explanation.message,
                    "%i%i%i%i",
                  ),
                  def_id,
                  pat_id,
                  def_id,
                  body_id,
                ),
                pat_coloring_ids
                @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
              );
            } else {
              basic(doc, LangDocMessages.let_listnil_exp_group, options);
            };
          } else {
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.let_listlit_exp_group,
                docs,
              );
            if (LangDocMessages.let_listlit_exp.id == doc.id) {
              let pat_id = List.nth(pat.ids, 0);
              let def_id = List.nth(def.ids, 0);
              let body_id = List.nth(body.ids, 0);
              let pat_coloring_ids =
                switch (List.nth(doc.syntactic_form, 0)) {
                | Tile(tile) => [
                    (
                      Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                      pat_id,
                    ),
                    (
                      Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                      def_id,
                    ),
                  ]
                | _ => []
                };
              get_message(
                doc,
                options,
                LangDocMessages.let_listlit_exp_group,
                Printf.sprintf(
                  Scanf.format_from_string(
                    doc.explanation.message,
                    "%i%i%i%i%i",
                  ),
                  def_id,
                  pat_id,
                  List.length(elements),
                  def_id,
                  body_id,
                ),
                pat_coloring_ids
                @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
              );
            } else {
              basic(doc, LangDocMessages.let_listlit_exp_group, options);
            };
          }
        | Cons(hd, tl) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.let_cons_exp_group,
              docs,
            );
          if (LangDocMessages.let_cons_exp.id == doc.id) {
            let hd_id = List.nth(hd.ids, 0);
            let tl_id = List.nth(tl.ids, 0);
            let def_id = List.nth(def.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) =>
                let pat = List.nth(tile.children, 0);
                [
                  (Piece.id(List.nth(pat, 0)), hd_id),
                  (Piece.id(List.nth(pat, 2)), tl_id),
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ];
              | _ => []
              };
            get_message(
              doc,
              options,
              LangDocMessages.let_cons_exp_group,
              Printf.sprintf(
                Scanf.format_from_string(
                  doc.explanation.message,
                  "%i%i%i%i%i%i%i",
                ),
                def_id,
                hd_id,
                tl_id,
                hd_id,
                tl_id,
                def_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            basic(doc, LangDocMessages.let_cons_exp_group, options);
          };
        | Var(var) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.let_var_exp_group,
              docs,
            );
          if (LangDocMessages.let_var_exp.id == doc.id) {
            let pat_id = List.nth(pat.ids, 0);
            let def_id = List.nth(def.ids, 0);
            let body_id = List.nth(body.ids, 0);
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 0)),
                    pat_id,
                  ),
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ]
              | _ => []
              };
            // TODO Make sure everywhere printing the float literal print it prettier
            get_message(
              doc,
              options,
              LangDocMessages.let_var_exp_group,
              Printf.sprintf(
                Scanf.format_from_string(doc.explanation.message, "%i%i%s%i"),
                def_id,
                pat_id,
                var,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          } else {
            /* TODO The coloring for the syntactic form is sometimes wrong here... */
            basic(
              doc,
              LangDocMessages.let_var_exp_group,
              options,
            );
          };
        | Tuple(elements) =>
          let pat_id = List.nth(pat.ids, 0);
          let def_id = List.nth(def.ids, 0);
          let body_id = List.nth(body.ids, 0);
          let basic_tuple = (doc: LangDocMessages.form, group_id, options) => {
            let pat_coloring_ids =
              switch (List.nth(doc.syntactic_form, 0)) {
              | Tile(tile) => [
                  (
                    Piece.id(List.nth(List.nth(tile.children, 0), 1)),
                    pat_id,
                  ),
                  (
                    Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                    def_id,
                  ),
                ]
              | _ => []
              };
            get_message(
              doc,
              options,
              group_id,
              Printf.sprintf(
                Scanf.format_from_string(
                  doc.explanation.message,
                  "%i%i%i%i%i%i",
                ),
                def_id,
                pat_id,
                List.length(elements),
                def_id,
                pat_id,
                body_id,
              ),
              pat_coloring_ids
              @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
            );
          };

          switch (List.length(elements)) {
          | 2 =>
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.let_tuple2_exp_group,
                docs,
              );
            if (LangDocMessages.let_tuple2_exp.id == doc.id) {
              let pat1_id = List.nth(List.nth(elements, 0).ids, 0);
              let pat2_id = List.nth(List.nth(elements, 1).ids, 0);
              let pat_coloring_ids =
                switch (List.nth(doc.syntactic_form, 0)) {
                | Tile(tile) =>
                  let pat = List.nth(tile.children, 0);
                  [
                    (Piece.id(List.nth(pat, 0)), pat1_id),
                    (Piece.id(List.nth(pat, 2)), pat2_id),
                    (
                      Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                      def_id,
                    ),
                  ];
                | _ => []
                };
              print_endline("Message: " ++ doc.explanation.message);
              get_message(
                doc,
                options,
                LangDocMessages.let_tuple2_exp_group,
                Printf.sprintf(
                  Scanf.format_from_string(
                    doc.explanation.message,
                    "%i%i%i%i%i",
                  ),
                  def_id,
                  pat1_id,
                  pat2_id,
                  def_id,
                  body_id,
                ),
                pat_coloring_ids
                @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
              );
            } else if (LangDocMessages.let_tuple_exp.id == doc.id) {
              basic_tuple(doc, LangDocMessages.let_tuple2_exp_group, options);
            } else {
              basic(doc, LangDocMessages.let_tuple2_exp_group, options);
            };
          | 3 =>
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.let_tuple3_exp_group,
                docs,
              );
            // TODO Syntactic form can go off page - so can examples
            if (LangDocMessages.let_tuple3_exp.id == doc.id) {
              let pat1_id = List.nth(List.nth(elements, 0).ids, 0);
              let pat2_id = List.nth(List.nth(elements, 1).ids, 0);
              let pat3_id = List.nth(List.nth(elements, 2).ids, 0);
              let pat_coloring_ids =
                switch (List.nth(doc.syntactic_form, 0)) {
                | Tile(tile) =>
                  let pat = List.nth(tile.children, 0);
                  [
                    (Piece.id(List.nth(pat, 0)), pat1_id),
                    (Piece.id(List.nth(pat, 2)), pat2_id),
                    (Piece.id(List.nth(pat, 4)), pat3_id),
                    (
                      Piece.id(List.nth(List.nth(tile.children, 1), 0)),
                      def_id,
                    ),
                  ];
                | _ => []
                };
              get_message(
                doc,
                options,
                LangDocMessages.let_tuple3_exp_group,
                Printf.sprintf(
                  Scanf.format_from_string(
                    doc.explanation.message,
                    "%i%i%i%i%i%i",
                  ),
                  def_id,
                  pat1_id,
                  pat2_id,
                  pat3_id,
                  def_id,
                  body_id,
                ),
                pat_coloring_ids
                @ [(Piece.id(List.nth(doc.syntactic_form, 1)), body_id)],
              );
            } else if (LangDocMessages.let_tuple_exp.id == doc.id) {
              basic_tuple(doc, LangDocMessages.let_tuple3_exp_group, options);
            } else {
              basic(doc, LangDocMessages.let_tuple3_exp_group, options);
            };
          | _ =>
            let (doc, options) =
              LangDocMessages.get_form_and_options(
                LangDocMessages.let_tuple_base_exp_group,
                docs,
              );
            if (LangDocMessages.let_tuple_exp.id == doc.id) {
              basic_tuple(
                doc,
                LangDocMessages.let_tuple_base_exp_group,
                options,
              );
            } else {
              basic(doc, LangDocMessages.let_tuple_base_exp_group, options);
            };
          };
        | Invalid(_) => default // Shouldn't get hit
        | Parens(_) => default // Shouldn't get hit?
        | TypeAnn(_) => default // Shouldn't get hit?
        };
      | Ap(fun_, arg) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.funapp_exp_group,
            docs,
          );
        let fun_id = List.nth(fun_.ids, 0);
        let arg_id = List.nth(arg.ids, 0);
        let coloring_ids =
          switch (List.nth(doc.syntactic_form, 1)) {
          | Tile(tile) => [
              (Piece.id(List.nth(List.nth(tile.children, 0), 0)), arg_id),
            ]
          | _ => []
          };
        get_message(
          doc,
          options,
          LangDocMessages.funapp_exp_group,
          Printf.sprintf(
            Scanf.format_from_string(doc.explanation.message, "%i%i"),
            fun_id,
            arg_id,
          ),
          [(Piece.id(List.nth(doc.syntactic_form, 0)), fun_id)]
          @ coloring_ids,
        );
      | If(cond, then_, else_) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.if_exp_group,
            docs,
          );
        let cond_id = List.nth(cond.ids, 0);
        let then_id = List.nth(then_.ids, 0);
        let else_id = List.nth(else_.ids, 0);
        let coloring_ids =
          switch (List.nth(doc.syntactic_form, 0)) {
          | Tile(tile) => [
              (Piece.id(List.nth(List.nth(tile.children, 0), 0)), cond_id),
              (Piece.id(List.nth(List.nth(tile.children, 1), 0)), then_id),
            ]
          | _ => []
          };
        get_message(
          doc,
          options,
          LangDocMessages.if_exp_group,
          Printf.sprintf(
            Scanf.format_from_string(doc.explanation.message, "%i%i%i"),
            cond_id,
            then_id,
            else_id,
          ),
          coloring_ids
          @ [(Piece.id(List.nth(doc.syntactic_form, 1)), else_id)],
        );
      | Seq(left, right) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.seq_exp_group,
            docs,
          );
        let left_id = List.nth(left.ids, 0);
        let right_id = List.nth(right.ids, 0);
        get_message(
          doc,
          options,
          LangDocMessages.seq_exp_group,
          Printf.sprintf(
            Scanf.format_from_string(doc.explanation.message, "%i%i"),
            left_id,
            right_id,
          ),
          [
            (Piece.id(List.nth(doc.syntactic_form, 0)), left_id),
            (Piece.id(List.nth(doc.syntactic_form, 2)), right_id),
          ],
        );
      | Test(body) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.test_group,
            docs,
          );
        let body_id = List.nth(body.ids, 0);
        let coloring_ids =
          switch (List.nth(doc.syntactic_form, 0)) {
          | Tile(tile) => [
              (Piece.id(List.nth(List.nth(tile.children, 0), 0)), body_id),
            ]
          | _ => []
          };
        get_message(
          doc,
          options,
          LangDocMessages.test_group,
          Printf.sprintf(
            Scanf.format_from_string(doc.explanation.message, "%i"),
            body_id,
          ),
          coloring_ids,
        );
      | Parens(term) => get_message_exp(term.term) // No Special message?
      | Cons(hd, tl) =>
        let (doc, options) =
          LangDocMessages.get_form_and_options(
            LangDocMessages.cons_exp_group,
            docs,
          );
        let hd_id = List.nth(hd.ids, 0);
        let tl_id = List.nth(tl.ids, 0);
        get_message(
          doc,
          options,
          LangDocMessages.cons_exp_group,
          Printf.sprintf(
            Scanf.format_from_string(doc.explanation.message, "%i%i"),
            hd_id,
            tl_id,
          ), // https://stackoverflow.com/questions/31998408/ocaml-converting-strings-to-a-unit-string-format
          [
            (Piece.id(List.nth(doc.syntactic_form, 0)), hd_id),
            (Piece.id(List.nth(doc.syntactic_form, 2)), tl_id),
          ],
        );
      | UnOp(op, exp) =>
        switch (op) {
        | Int(Minus) =>
          let (doc, options) =
            LangDocMessages.get_form_and_options(
              LangDocMessages.int_unary_minus_group,
              docs,
            );
          let exp_id = List.nth(exp.ids, 0);
          get_message(
            doc,
            options,
            LangDocMessages.int_unary_minus_group,
            Printf.sprintf(
              Scanf.format_from_string(doc.explanation.message, "%i"),
              exp_id,
            ),
            [(Piece.id(List.nth(doc.syntactic_form, 1)), exp_id)],
          );
        }
      | BinOp(op, left, right) =>
        let group =
          switch (op) {
          | Int(Plus) => LangDocMessages.int_plus_group
          | Int(Minus) => LangDocMessages.int_minus_group
          | Int(Times) => LangDocMessages.int_times_group
          | Int(Divide) => LangDocMessages.int_divide_group
          | Int(LessThan) => LangDocMessages.int_lt_group
          | Int(GreaterThan) => LangDocMessages.int_gt_group
          | Int(Equals) => LangDocMessages.int_eq_group
          | Float(Plus) => LangDocMessages.float_plus_group
          | Float(Minus) => LangDocMessages.float_minus_group
          | Float(Times) => LangDocMessages.float_times_group
          | Float(Divide) => LangDocMessages.float_divide_group
          | Float(LessThan) => LangDocMessages.float_lt_group
          | Float(GreaterThan) => LangDocMessages.float_gt_group
          | Float(Equals) => LangDocMessages.float_eq_group
          | Bool(And) => LangDocMessages.bool_and_group
          | Bool(Or) => LangDocMessages.bool_or_group
          };
        let (doc, options) =
          LangDocMessages.get_form_and_options(group, docs);
        let left_id = List.nth(left.ids, 0);
        let right_id = List.nth(right.ids, 0);
        get_message(
          doc,
          options,
          group,
          Printf.sprintf(
            Scanf.format_from_string(doc.explanation.message, "%i%i"),
            left_id,
            right_id,
          ),
          [
            (Piece.id(List.nth(doc.syntactic_form, 0)), left_id),
            (Piece.id(List.nth(doc.syntactic_form, 2)), right_id),
          ],
        );
      | Match(_scrut, _rules) => default
      };
    get_message_exp(term.term);
  | Some(InfoPat(_))
  | Some(InfoTyp(_))
  | Some(InfoRul(_))
  | None
  | Some(Invalid(_)) => default
  };
};

let section = (~section_clss: string, ~title: string, contents: Node.t) =>
  div(
    ~attr=clss(["section", section_clss]),
    [div(~attr=clss(["section-title"]), [text(title)]), contents],
  );

let get_color_map =
    (
      ~inject,
      ~font_metrics,
      ~settings: Model.settings,
      ~doc: LangDocMessages.t,
      index': option(int),
      info_map: Statics.map,
    ) => {
  let info: option(Statics.t) =
    switch (index') {
    | Some(index) =>
      switch (Id.Map.find_opt(index, info_map)) {
      | Some(ci) => Some(ci)
      | None => None
      }
    | None => None
    };
  let (_, (_, color_map), _) =
    get_doc(~inject, ~font_metrics, ~settings, ~docs=doc, info);
  print_endline("GOT THE COLORS");
  color_map;
};

let view =
    (
      ~inject,
      ~font_metrics: FontMetrics.t,
      ~settings: Model.settings,
      ~doc: LangDocMessages.t,
      index': option(int),
      info_map: Statics.map,
    ) => {
  let info: option(Statics.t) =
    switch (index') {
    | Some(index) =>
      switch (Id.Map.find_opt(index, info_map)) {
      | Some(ci) => Some(ci)
      | None => None
      }
    | None => None
    };
  // TODO Make sure code examples aren't flowing off the page
  print_endline("TRYING TO GET THE DOC");
  let (syn_form, (explanation, _), example) =
    get_doc(~inject, ~font_metrics, ~settings, ~docs=doc, info);
  print_endline("GOT THE DOC");
  div(
    ~attr=clss(["lang-doc"]),
    [
      div([
        toggle("🔆", doc.highlight, _ =>
          inject(
            Update.UpdateLangDocMessages(LangDocMessages.ToggleHighlight),
          )
        ),
        section(
          ~section_clss="syntactic-form",
          ~title="Syntactic Form",
          syn_form,
        ),
        section(
          ~section_clss="explanation",
          ~title="Explanation",
          explanation,
        ),
        section(~section_clss="examples", ~title="Examples", example),
      ]),
    ],
  );
};
