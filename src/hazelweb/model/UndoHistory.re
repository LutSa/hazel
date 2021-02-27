open Sexplib.Std;

[@deriving sexp]
type undo_history_entry = {
  /* cardstacks after non-movement action applied */
  cardstacks_after_action: ZCardstacks.t,
  /* cardstacks_after_move is initially the same as cardstacks_after_action.
     if there is a movement action, update it. */
  cardstacks_after_move: ZCardstacks.t,
  cursor_term_info: UndoHistoryCore.cursor_term_info,
  previous_action: Action.t,
  action_group: UndoHistoryCore.action_group,
  timestamp: UndoHistoryCore.timestamp,
};

[@deriving sexp]
type undo_history_group = {
  group_entries: ZList.t(undo_history_entry, undo_history_entry),
  is_expanded: bool,
};

[@deriving sexp]
type t = {
  groups: ZList.t(undo_history_group, undo_history_group),
  all_hidden_history_expand: bool,
  /* history panel automatically scrolls current entry into view,
     but this behavior should be disabled when user is hovering over panel */
  disable_auto_scrolling: bool,
  preview_on_hover: bool,
  hover_recover_group_id: int,
  hover_recover_elt_id: int,
  cur_group_id: int,
  cur_elt_id: int,
};

// TODO refactor undo history so that it contains
// a sequence of `ZExp.t` entries (or whatever is
// the constructor arg to `Program.Focused`)
exception EntryWithoutCursor;

let update_disable_auto_scrolling = (disable_auto_scrolling: bool, history: t) => {
  {...history, disable_auto_scrolling};
};

let disable_redo = (undo_history: t): bool => {
  /* if there is no entry after current entry,
     then history is at the latest entry. redo should be disabled */
  List.length(ZList.prj_prefix(undo_history.groups)) == 0
  && List.length(
       ZList.prj_prefix(ZList.prj_z(undo_history.groups).group_entries),
     )
  == 0;
};
let disable_undo = (undo_history: t): bool => {
  /* if there is no entry before current entry,
     then history is at the initial entry, undo should be disabled */
  List.length(ZList.prj_suffix(undo_history.groups)) == 0
  && List.length(
       ZList.prj_suffix(ZList.prj_z(undo_history.groups).group_entries),
     )
  == 0;
};

let get_cardstacks = (history: t, ~is_after_move: bool): ZCardstacks.t => {
  let cur_entry = ZList.prj_z(ZList.prj_z(history.groups).group_entries);
  if (is_after_move) {
    cur_entry.cardstacks_after_move;
  } else {
    cur_entry.cardstacks_after_action;
  };
};

let push_history_entry =
    (
      ~prev_group: undo_history_group,
      ~new_entry: undo_history_entry,
      ~is_expanded: bool,
    )
    : undo_history_group => {
  {
    group_entries: (
      [],
      new_entry,
      [
        ZList.prj_z(prev_group.group_entries),
        ...ZList.prj_suffix(prev_group.group_entries),
      ],
    ),
    is_expanded,
  };
};

/* return true if caret jump to another term when new action applied */
let caret_jump =
    (prev_group: undo_history_group, new_cardstacks_before: ZCardstacks.t)
    : bool => {
  let prev_entry = ZList.prj_z(prev_group.group_entries);
  let (prev_steps, _) =
    prev_entry.cardstacks_after_action
    |> ZCardstacks.get_program
    |> Program.get_path
    |> OptUtil.get(() => raise(EntryWithoutCursor));
  let (new_steps, _) =
    new_cardstacks_before
    |> ZCardstacks.get_program
    |> Program.get_path
    |> OptUtil.get(() => raise(EntryWithoutCursor));
  prev_steps != new_steps;
};

/* return true if new entry can be grouped into the previous group */
let group_entry =
    (
      ~prev_group: undo_history_group,
      ~new_cardstacks_before: ZCardstacks.t,
      ~new_action_group: UndoHistoryCore.action_group,
    )
    : bool => {
  let prev_entry = ZList.prj_z(prev_group.group_entries);
  UndoHistoryCore.group_action_group(
    prev_entry.action_group,
    new_action_group,
  )
  && !caret_jump(prev_group, new_cardstacks_before);
};

let get_initial_entry_in_group =
    (undo_history_group: undo_history_group): option(undo_history_entry) => {
  switch (List.rev(ZList.join(undo_history_group.group_entries))) {
  | [] => None
  | [head, ..._] => Some(head)
  };
};

let get_delete_action_group =
    (
      prev_group: undo_history_group,
      new_cardstacks_before: ZCardstacks.t,
      new_cursor_term_info: UndoHistoryCore.cursor_term_info,
    )
    : UndoHistoryCore.action_group =>
  if (group_entry(
        ~prev_group,
        ~new_cardstacks_before,
        ~new_action_group=
          DeleteEdit(Term(new_cursor_term_info.cursor_term_before, true)),
      )) {
    let prev_entry = ZList.prj_z(prev_group.group_entries);
    if (UndoHistoryCore.is_var_group(prev_entry.action_group)) {
      /* if delete group start from var edition, the deleted term is the initial term in this group */
      switch (get_initial_entry_in_group(prev_group)) {
      | None =>
        DeleteEdit(Term(new_cursor_term_info.cursor_term_before, false))
      | Some(initial_entry) =>
        DeleteEdit(
          Term(initial_entry.cursor_term_info.cursor_term_before, false),
        )
      };
    } else if (UndoHistoryCore.is_var_insert(prev_entry.action_group)) {
      let cursor_term_len_larger = UndoHistoryCore.cursor_term_len_larger;
      /* if delete group start from var insertion, the deleted term is the longest term in this group */
      let rec max_len_term =
              (
                ls: list(undo_history_entry),
                cursor_term: UndoHistoryCore.cursor_term,
              )
              : UndoHistoryCore.cursor_term => {
        switch (ls) {
        | [] => cursor_term
        | [elt] =>
          switch (
            elt.cursor_term_info.cursor_term_before,
            elt.cursor_term_info.cursor_term_after,
          ) {
          | (Exp(_, exp_1), Exp(_, exp_2)) =>
            switch (exp_1, exp_2) {
            | (Var(_, _, _), Var(_, _, _))
            | (IntLit(_, _), IntLit(_, _))
            | (FloatLit(_, _), FloatLit(_, _))
            | (BoolLit(_, _), BoolLit(_, _)) =>
              cursor_term_len_larger(
                cursor_term,
                cursor_term_len_larger(
                  elt.cursor_term_info.cursor_term_after,
                  elt.cursor_term_info.cursor_term_before,
                ),
              )
            | _ =>
              cursor_term_len_larger(
                cursor_term,
                elt.cursor_term_info.cursor_term_after,
              )
            }
          | (Pat(_, pat_1), Pat(_, pat_2)) =>
            switch (pat_1, pat_2) {
            | (Var(_, _, _), Var(_, _, _))
            | (IntLit(_, _), IntLit(_, _))
            | (FloatLit(_, _), FloatLit(_, _))
            | (BoolLit(_, _), BoolLit(_, _)) =>
              cursor_term_len_larger(
                cursor_term,
                cursor_term_len_larger(
                  elt.cursor_term_info.cursor_term_after,
                  elt.cursor_term_info.cursor_term_before,
                ),
              )
            | _ =>
              cursor_term_len_larger(
                cursor_term,
                elt.cursor_term_info.cursor_term_after,
              )
            }
          | _ =>
            cursor_term_len_larger(
              cursor_term,
              elt.cursor_term_info.cursor_term_after,
            )
          }
        | [head, ...tail] =>
          let larger_term =
            cursor_term_len_larger(
              cursor_term,
              head.cursor_term_info.cursor_term_after,
            );
          max_len_term(tail, larger_term);
        };
      };
      DeleteEdit(
        Term(
          max_len_term(
            [
              ZList.prj_z(prev_group.group_entries),
              ...ZList.prj_suffix(prev_group.group_entries),
            ],
            new_cursor_term_info.cursor_term_before,
          ),
          true,
        ),
      );
    } else {
      DeleteEdit(Term(new_cursor_term_info.cursor_term_before, false));
    };
  } else {
    DeleteEdit(Term(new_cursor_term_info.cursor_term_before, false));
  };

let delete_group =
    (
      ~prev_group: undo_history_group,
      ~new_cardstacks_before: ZCardstacks.t,
      ~new_cursor_term_info: UndoHistoryCore.cursor_term_info,
    )
    : option(UndoHistoryCore.action_group) =>
  if (CursorInfo_common.is_empty_line(new_cursor_term_info.cursor_term_before)) {
    Some(DeleteEdit(EmptyLine));
  } else if (CursorInfo_common.is_empty_line(
               new_cursor_term_info.cursor_term_after,
             )
             || CursorInfo_common.is_empty_hole(
                  new_cursor_term_info.cursor_term_after,
                )) {
    /* if the term becomes hole or empty line, the action group is deleting the whole term */
    Some(
      get_delete_action_group(
        prev_group,
        new_cardstacks_before,
        new_cursor_term_info,
      ),
    );
  } else {
    /* the action group is editing the term */
    let prev_entry = ZList.prj_z(prev_group.group_entries);
    if (!caret_jump(prev_group, new_cardstacks_before)
        && UndoHistoryCore.is_var_insert(prev_entry.action_group)) {
      Some(VarGroup(Insert(new_cursor_term_info.cursor_term_after)));
    } else {
      let initial_term =
        if (group_entry(
              ~prev_group,
              ~new_cardstacks_before,
              ~new_action_group=
                VarGroup(
                  Edit({
                    start_from: new_cursor_term_info.cursor_term_before,
                    end_with: new_cursor_term_info.cursor_term_after,
                  }),
                ),
            )) {
          switch (get_initial_entry_in_group(prev_group)) {
          | None => new_cursor_term_info.cursor_term_before
          | Some(initial_entry) =>
            initial_entry.cursor_term_info.cursor_term_before
          };
        } else {
          new_cursor_term_info.cursor_term_before;
        };
      Some(
        VarGroup(
          Edit({
            start_from: initial_term,
            end_with: new_cursor_term_info.cursor_term_after,
          }),
        ),
      );
    };
  };
let delim_edge_handle =
    (
      ~new_cursor_term_info: UndoHistoryCore.cursor_term_info,
      ~adjacent_is_empty_line: bool,
    )
    : option(UndoHistoryCore.action_group) =>
  if (adjacent_is_empty_line) {
    /* delete adjacent empty line */
    Some(DeleteEdit(EmptyLine));
  } else if (CursorInfo_common.is_empty_hole(
               new_cursor_term_info.cursor_term_before,
             )) {
    /* delete space */
    Some(DeleteEdit(Space));
  } else {
    None;
        /* jump to next term */
  };
let delete =
    (
      ~prev_group: undo_history_group,
      ~new_cardstacks_before: ZCardstacks.t,
      ~new_cursor_term_info: UndoHistoryCore.cursor_term_info,
    )
    : option(UndoHistoryCore.action_group) => {
  let cursor_pos =
    UndoHistoryCore.get_cursor_pos(new_cursor_term_info.cursor_term_before);

  switch (cursor_pos) {
  | OnText(_) =>
    if (CursorInfo_Exp.caret_is_after_zoperand(
          new_cursor_term_info.zexp_before,
        )) {
      delim_edge_handle(
        ~new_cursor_term_info,
        ~adjacent_is_empty_line=new_cursor_term_info.next_is_empty_line,
      );
    } else {
      //edit var
      delete_group(
        ~prev_group,
        ~new_cardstacks_before,
        ~new_cursor_term_info,
      );
    }
  | OnDelim(pos, side) =>
    switch (side) {
    | Before =>
      if (pos == 1
          && UndoHistoryCore.has_typ_ann(
               new_cursor_term_info.cursor_term_before,
             )) {
        /* num==1 is the position of ':' in an expression */
        Some(
          DeleteEdit(TypeAnn),
        );
      } else {
        /* delete the whole term */
        Some(
          get_delete_action_group(
            prev_group,
            new_cardstacks_before,
            new_cursor_term_info,
          ),
        );
      }
    | After =>
      delim_edge_handle(
        ~new_cursor_term_info,
        ~adjacent_is_empty_line=new_cursor_term_info.next_is_empty_line,
      )
    }
  | OnOp(side) =>
    switch (side) {
    | Before =>
      /* delete and reach a hole */
      Some(
        get_delete_action_group(
          prev_group,
          new_cardstacks_before,
          new_cursor_term_info,
        ),
      )
    | After =>
      /* move caret to next term, just ignore this move */
      None
    }
  };
};

let backspace =
    (
      ~prev_group: undo_history_group,
      ~new_cardstacks_before: ZCardstacks.t,
      ~new_cursor_term_info: UndoHistoryCore.cursor_term_info,
    )
    : option(UndoHistoryCore.action_group) => {
  let cursor_pos =
    UndoHistoryCore.get_cursor_pos(new_cursor_term_info.cursor_term_before);
  switch (cursor_pos) {
  | OnText(_) =>
    if (CursorInfo_Exp.caret_is_before_zoperand(
          new_cursor_term_info.zexp_before,
        )) {
      delim_edge_handle(
        ~new_cursor_term_info,
        ~adjacent_is_empty_line=new_cursor_term_info.prev_is_empty_line,
      );
    } else {
      //edit var
      delete_group(
        ~prev_group,
        ~new_cardstacks_before,
        ~new_cursor_term_info,
      );
    }
  | OnDelim(pos, side) =>
    switch (side) {
    | Before =>
      delim_edge_handle(
        ~new_cursor_term_info,
        ~adjacent_is_empty_line=new_cursor_term_info.prev_is_empty_line,
      )
    | After =>
      if (pos == 1
          && UndoHistoryCore.has_typ_ann(
               new_cursor_term_info.cursor_term_before,
             )) {
        /* num==1 is the position of ':' in an expression */
        Some(
          DeleteEdit(TypeAnn),
        );
      } else {
        /* delete the whole term */
        Some(
          get_delete_action_group(
            prev_group,
            new_cardstacks_before,
            new_cursor_term_info,
          ),
        );
      }
    }
  | OnOp(side) =>
    switch (side) {
    | Before =>
      /* move caret to next term, just ignore this move */
      None
    | After =>
      /* delete and reach a hole */
      Some(
        get_delete_action_group(
          prev_group,
          new_cardstacks_before,
          new_cursor_term_info,
        ),
      )
    }
  };
};

let get_new_action_group =
    (
      ~prev_group: undo_history_group,
      ~new_cardstacks_before: ZCardstacks.t,
      ~new_cursor_term_info: UndoHistoryCore.cursor_term_info,
      ~action: Action.t,
    )
    : option(UndoHistoryCore.action_group) =>
  if (UndoHistoryCore.is_move_action(new_cursor_term_info)) {
    None;
        /* It's a caret movement */
  } else {
    switch (action) {
    | Delete =>
      delete(~prev_group, ~new_cardstacks_before, ~new_cursor_term_info)
    | Backspace =>
      backspace(~prev_group, ~new_cardstacks_before, ~new_cursor_term_info)
    | Construct(shape) =>
      switch (shape) {
      | SLine =>
        switch (
          CursorInfo_Exp.get_outer_zrules(new_cursor_term_info.zexp_before)
        ) {
        | None => Some(ConstructEdit(shape))
        | Some(zrules_before) =>
          switch (
            CursorInfo_Exp.get_outer_zrules(new_cursor_term_info.zexp_after)
          ) {
          | None => Some(ConstructEdit(shape))
          | Some(zrules_after) =>
            if (ZList.length(zrules_before) < ZList.length(zrules_after)) {
              Some(CaseRule);
            } else {
              Some(ConstructEdit(shape));
            }
          }
        }
      | SCommentLine
      | SParenthesized
      | SList
      | SAnn
      | SLam
      | SListNil
      | SInj(_)
      | SLet
      | SLivelitDef
      | SCase => Some(ConstructEdit(shape))
      | SLeftBracket =>
        switch (new_cursor_term_info.cursor_term_before) {
        | Exp(OnDelim(_, After), _)
        | Pat(OnDelim(_, After), _) => Some(Subscript)
        | _ => Some(ConstructEdit(shape))
        }
      | SQuote
      | SChar(_) =>
        if (group_entry(
              ~prev_group,
              ~new_cardstacks_before,
              ~new_action_group=
                VarGroup(
                  Edit({
                    start_from: new_cursor_term_info.cursor_term_before,
                    end_with: new_cursor_term_info.cursor_term_after,
                  }),
                ),
            )) {
          switch (get_initial_entry_in_group(prev_group)) {
          | None =>
            Some(VarGroup(Insert(new_cursor_term_info.cursor_term_after)))
          | Some(initial_entry) =>
            if (UndoHistoryCore.is_var_insert(initial_entry.action_group)) {
              Some(
                VarGroup(Insert(new_cursor_term_info.cursor_term_after)),
              );
            } else {
              Some(
                VarGroup(
                  Edit({
                    start_from:
                      initial_entry.cursor_term_info.cursor_term_before,
                    end_with: new_cursor_term_info.cursor_term_after,
                  }),
                ),
              );
            }
          };
        } else if (CursorInfo_common.is_empty_hole(
                     new_cursor_term_info.cursor_term_before,
                   )
                   || CursorInfo_common.is_empty_line(
                        new_cursor_term_info.cursor_term_before,
                      )
                   || !
                        CursorInfo_common.cursor_term_is_editable(
                          new_cursor_term_info.cursor_term_before,
                        )) {
          Some(VarGroup(Insert(new_cursor_term_info.cursor_term_after)));
        } else {
          Some(
            VarGroup(
              Edit({
                start_from: new_cursor_term_info.cursor_term_before,
                end_with: new_cursor_term_info.cursor_term_after,
              }),
            ),
          );
        }

      | SOp(op) =>
        switch (op) {
        | SMinus
        | SPlus
        | STimes
        | SDivide
        | SLessThan
        | SGreaterThan
        | SEquals
        | SComma
        | SArrow
        | SVBar
        | SCons
        | SAnd
        | SOr
        | SCaret => Some(ConstructEdit(shape))
        | SSpace =>
          switch (new_cursor_term_info.cursor_term_before) {
          | Exp(_, Var(_, InVarHole(Keyword(k), _), _)) =>
            let pos_before =
              UndoHistoryCore.get_cursor_pos(
                new_cursor_term_info.cursor_term_before,
              );
            switch (k, pos_before) {
            | (Let, OnText(3)) => Some(ConstructEdit(SLet))
            | (Case, OnText(4)) => Some(ConstructEdit(SCase))
            | (LivelitDef, OnText(7)) => Some(ConstructEdit(SLivelitDef))
            | _ => Some(ConstructEdit(SOp(SSpace)))
            };
          | Exp(OnText(index), Var(_, _, var)) =>
            let (left_var, _) = Var.split(index, var);
            if (Var.is_let(left_var)) {
              Some(ConstructEdit(SLet));
            } else if (Var.is_case(left_var)) {
              Some(ConstructEdit(SCase));
            } else {
              Some(ConstructEdit(SOp(SSpace)));
            };
          | _ => Some(ConstructEdit(SOp(SSpace)))
          }
        }
      }
    | SwapUp => Some(SwapEdit(Up))
    | SwapDown => Some(SwapEdit(Down))
    | SwapLeft => Some(SwapEdit(Left))
    | SwapRight => Some(SwapEdit(Right))
    | MoveTo(_)
    | MoveLeft
    | MoveRight
    | MoveToNextHole
    | MoveToPrevHole
    | Init
    | PerformLivelitAction(_) => None
    };
  };
let get_cursor_term_info =
    (
      ~new_cardstacks_after: ZCardstacks.t,
      ~new_cardstacks_before: ZCardstacks.t,
    )
    : UndoHistoryCore.cursor_term_info => {
  let zexp_before =
    new_cardstacks_before
    |> ZCardstacks.get_program
    |> Program.get_zexp
    |> OptUtil.get(() => raise(EntryWithoutCursor));
  let (prev_is_empty_line, next_is_empty_line) =
    CursorInfo_Exp.adjacent_is_emptyline(zexp_before);
  let cursor_info_before =
    new_cardstacks_before
    |> ZCardstacks.get_program
    |> Program.get_cursor_info
    |> OptUtil.get(() => raise(EntryWithoutCursor));
  let cursor_term_before = cursor_info_before.cursor_term;
  let zexp_after =
    new_cardstacks_after
    |> ZCardstacks.get_program
    |> Program.get_zexp
    |> OptUtil.get(() => raise(EntryWithoutCursor));
  let cursor_info_after =
    new_cardstacks_after
    |> ZCardstacks.get_program
    |> Program.get_cursor_info
    |> OptUtil.get(() => raise(EntryWithoutCursor));
  let cursor_term_after = cursor_info_after.cursor_term;

  {
    cursor_term_before,
    cursor_term_after,
    zexp_before,
    zexp_after,
    prev_is_empty_line,
    next_is_empty_line,
  };
};

let push_edit_state =
    (undo_history: t, new_cardstacks_after: ZCardstacks.t, action: Action.t)
    : t => {
  let prev_group = ZList.prj_z(undo_history.groups);
  let new_cardstacks_before =
    get_cardstacks(
      undo_history,
      ~is_after_move=
        switch (action) {
        | MoveTo(_)
        | MoveLeft
        | MoveRight
        | MoveToNextHole
        | MoveToPrevHole => true
        | _ => false
        },
    );
  let new_cursor_term_info =
    get_cursor_term_info(~new_cardstacks_before, ~new_cardstacks_after);
  let new_action_group =
    get_new_action_group(
      ~prev_group,
      ~new_cardstacks_before,
      ~new_cursor_term_info,
      ~action,
    );
  let timestamp = Unix.time();
  switch (new_action_group) {
  | None =>
    let prev_entry = ZList.prj_z(prev_group.group_entries);
    let new_entry = {
      ...prev_entry,
      cardstacks_after_move: new_cardstacks_after,
    };

    let new_group = {
      ...prev_group,
      group_entries: ZList.replace_z(new_entry, prev_group.group_entries),
    };
    {
      ...undo_history,
      groups: ZList.replace_z(new_group, undo_history.groups),
      disable_auto_scrolling: false,
    };
  | Some(new_action_group) =>
    let new_entry = {
      cardstacks_after_action: new_cardstacks_after,
      cardstacks_after_move: new_cardstacks_after,
      cursor_term_info: new_cursor_term_info,
      previous_action: action,
      action_group: new_action_group,
      timestamp,
    };
    if (group_entry(~prev_group, ~new_cardstacks_before, ~new_action_group)) {
      let new_group =
        push_history_entry(
          ~prev_group,
          ~new_entry,
          ~is_expanded=undo_history.all_hidden_history_expand,
        );
      {
        ...undo_history,
        groups: ([], new_group, ZList.prj_suffix(undo_history.groups)),
        disable_auto_scrolling: false,
        hover_recover_group_id: 0,
        hover_recover_elt_id: 0,
        cur_group_id: 0,
        cur_elt_id: 0,
      };
    } else {
      let new_group = {
        group_entries: ([], new_entry, []),
        is_expanded: undo_history.all_hidden_history_expand,
      };
      /* the successor entries should be cleared in previous group */
      let prev_group' = {
        ...prev_group,
        group_entries: (
          [],
          ZList.prj_z(prev_group.group_entries),
          ZList.prj_suffix(prev_group.group_entries),
        ),
      };
      {
        ...undo_history,
        groups: (
          [],
          new_group,
          [prev_group', ...ZList.prj_suffix(undo_history.groups)],
        ),
        disable_auto_scrolling: false,
        hover_recover_group_id: 0,
        hover_recover_elt_id: 0,
        cur_group_id: 0,
        cur_elt_id: 0,
      };
    };
  };
};

let update_groups =
    (
      new_groups: ZList.t(undo_history_group, undo_history_group),
      undo_history: t,
    )
    : t => {
  {
    ...undo_history,
    groups: new_groups,
    hover_recover_group_id: List.length(ZList.prj_prefix(new_groups)),
    hover_recover_elt_id:
      List.length(ZList.prj_prefix(ZList.prj_z(new_groups).group_entries)),
    cur_group_id: List.length(ZList.prj_prefix(new_groups)),
    cur_elt_id:
      List.length(ZList.prj_prefix(ZList.prj_z(new_groups).group_entries)),
  };
};
let shift_to_prev = (history: t): t => {
  let cur_group = ZList.prj_z(history.groups);
  /* shift to the previous state in the same group */
  switch (ZList.shift_next(cur_group.group_entries)) {
  | None =>
    /* if current group doesn't have previous state, shfit to the previous group */
    switch (ZList.shift_next(history.groups)) {
    | None => history
    | Some(new_groups) =>
      let new_group = ZList.prj_z(new_groups);
      let new_entries = ZList.shift_begin(new_group.group_entries);

      let new_group' = {
        /* is_expanded=true because the selected group should be expanded */
        group_entries: new_entries,
        is_expanded: true,
      };
      let new_groups = ZList.replace_z(new_group', new_groups);
      update_groups(new_groups, history);
    }
  | Some(new_entries) =>
    let new_group = {
      /* is_expanded=true because the selected group should be expanded */
      group_entries: new_entries,
      is_expanded: true,
    };
    let new_groups = ZList.replace_z(new_group, history.groups);
    update_groups(new_groups, history);
  };
};

let shift_to_next = (history: t): t => {
  let cur_group = ZList.prj_z(history.groups);
  /* shift to the previous state in the same group */
  switch (ZList.shift_prev(cur_group.group_entries)) {
  | None =>
    /* if current group doesn't have previous state, shfit to the previous group */
    switch (ZList.shift_prev(history.groups)) {
    | None => history
    | Some(new_groups) =>
      let new_group = ZList.prj_z(new_groups);
      let new_entries = ZList.shift_end(new_group.group_entries);
      let new_group' = {
        /* is_expanded=true because the selected group should be expanded */
        group_entries: new_entries,
        is_expanded: true,
      };
      let new_groups = ZList.replace_z(new_group', new_groups);
      update_groups(new_groups, history);
    }
  | Some(new_entries) =>
    let new_group = {
      /* is_expanded=true because the selected group should be expanded */
      group_entries: new_entries,
      is_expanded: true,
    };
    let new_groups = ZList.replace_z(new_group, history.groups);
    update_groups(new_groups, history);
  };
};

let shift_history =
    (group_id: int, elt_id: int, is_mouseenter: bool, undo_history: t): t => {
  switch (ZList.shift_to(group_id, undo_history.groups)) {
  | None => failwith("Impossible match, because undo_history is non-empty")
  | Some(new_groups) =>
    let cur_group = ZList.prj_z(new_groups);
    /* shift to the element with elt_id */
    switch (ZList.shift_to(elt_id, cur_group.group_entries)) {
    | None => failwith("Impossible because group_entries is non-empty")
    | Some(new_group_entries) =>
      let (cur_group_id, cur_elt_id) = (group_id, elt_id);
      let (hover_recover_group_id, hover_recover_elt_id) =
        if (is_mouseenter) {
          (
            undo_history.hover_recover_group_id,
            undo_history.hover_recover_elt_id,
          );
        } else {
          (group_id, elt_id);
        };
      {
        ...undo_history,
        groups:
          ZList.replace_z(
            {...cur_group, group_entries: new_group_entries},
            new_groups,
          ),
        disable_auto_scrolling: true,
        hover_recover_group_id,
        hover_recover_elt_id,
        cur_group_id,
        cur_elt_id,
      };
    };
  };
};

let toggle_all_hidden_history = (undo_history: t): t => {
  let hidden_group = (group: undo_history_group) => {
    ...group,
    is_expanded: !undo_history.all_hidden_history_expand,
  };
  {
    ...undo_history,
    groups: (
      List.map(hidden_group, ZList.prj_prefix(undo_history.groups)),
      hidden_group(ZList.prj_z(undo_history.groups)),
      List.map(hidden_group, ZList.prj_suffix(undo_history.groups)),
    ),
    all_hidden_history_expand: !undo_history.all_hidden_history_expand,
  };
};
