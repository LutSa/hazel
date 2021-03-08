open Sexplib.Std;

[@deriving sexp]
type step_status =
  | InvalidInput(int)
  | BoxedValue(DHExp.t)
  | Indet(DHExp.t)
  | Step(DHExp.t);

let rec step_evaluate = (d: DHExp.t): step_status =>
  switch (d) {
  | BoundVar(_) => InvalidInput(1)
  | Let(dp, d1, d2) =>
    switch (step_evaluate(d1)) {
    | Step(d1') => Step(Let(dp, d1', d2))
    | InvalidInput(msg) => InvalidInput(msg)
    | BoxedValue(d1)
    | Indet(d1) =>
      switch (Elaborator_Exp.matches(dp, d1)) {
      | Indet => Indet(d)
      | DoesNotMatch => Indet(d)
      | Matches(env) => Step(Elaborator_Exp.subst(env, d2))
      }
    }
  | FixF(x, _, d1) => Step(Elaborator_Exp.subst_var(d, x, d1))
  | Lam(_, _, _) => BoxedValue(d)
  | Ap(d1, d2) =>
    switch (step_evaluate(d1)) {
    | Step(d1') => Step(Ap(d1', d2))
    | InvalidInput(msg) => InvalidInput(msg)
    | BoxedValue(Lam(dp, _, d3)) =>
      switch (step_evaluate(d2)) {
      | Step(d2') => Step(Ap(d1, d2'))
      | InvalidInput(msg) => InvalidInput(msg)
      | BoxedValue(d2)
      | Indet(d2) =>
        switch (Elaborator_Exp.matches(dp, d2)) {
        | DoesNotMatch => Indet(d)
        | Indet => Indet(d)
        | Matches(env) =>
          /* beta rule */
          Step(Elaborator_Exp.subst(env, d3))
        }
      }
    | BoxedValue(Cast(d1', Arrow(ty1, ty2), Arrow(ty1', ty2')))
    | Indet(Cast(d1', Arrow(ty1, ty2), Arrow(ty1', ty2'))) =>
      switch (step_evaluate(d2)) {
      | Step(d2') => Step(Ap(d1, d2'))
      | InvalidInput(msg) => InvalidInput(msg)
      | BoxedValue(d2')
      | Indet(d2') =>
        /* ap cast rule */
        Step(Cast(Ap(d1', Cast(d2', ty1', ty1)), ty2, ty2'))
      }
    | BoxedValue(_) => InvalidInput(2)
    | Indet(d1') =>
      switch (step_evaluate(d2)) {
      | Step(d2') => Step(Ap(d1, d2'))
      | InvalidInput(msg) => InvalidInput(msg)
      | BoxedValue(d2')
      | Indet(d2') => Indet(Ap(d1', d2'))
      }
    }
  | ListNil(_)
  | BoolLit(_)
  | IntLit(_)
  | FloatLit(_)
  | Triv => BoxedValue(d)
  | BinBoolOp(op, d1, d2) =>
    switch (step_evaluate(d1)) {
    | Step(d1') => Step(BinBoolOp(op, d1', d2))
    | InvalidInput(msg) => InvalidInput(msg)
    | BoxedValue(BoolLit(b1) as d1') =>
      switch (step_evaluate(d2)) {
      | Step(d2') => Step(BinBoolOp(op, d1, d2'))
      | InvalidInput(msg) => InvalidInput(msg)
      | BoxedValue(BoolLit(b2)) =>
        BoxedValue(EvaluatorCommon.eval_bin_bool_op(op, b1, b2))
      | BoxedValue(_) => InvalidInput(3)
      | Indet(d2') => Indet(BinBoolOp(op, d1', d2'))
      }
    | BoxedValue(_) => InvalidInput(4)
    | Indet(d1') =>
      switch (step_evaluate(d2)) {
      | Step(d2') => Step(BinBoolOp(op, d1, d2'))
      | InvalidInput(msg) => InvalidInput(msg)
      | BoxedValue(d2')
      | Indet(d2') => Indet(BinBoolOp(op, d1', d2'))
      }
    }
  | BinIntOp(op, d1, d2) =>
    switch (step_evaluate(d1)) {
    | Step(d1') => Step(BinIntOp(op, d1', d2))
    | InvalidInput(msg) => InvalidInput(msg)
    | BoxedValue(IntLit(n1) as d1') =>
      switch (step_evaluate(d2)) {
      | Step(d2') => Step(BinIntOp(op, d1, d2'))
      | InvalidInput(msg) => InvalidInput(msg)
      | BoxedValue(IntLit(n2)) =>
        switch (op, n1, n2) {
        | (Divide, _, 0) =>
          Indet(
            InvalidOperation(
              BinIntOp(op, IntLit(n1), IntLit(n2)),
              DivideByZero,
            ),
          )
        | _ => BoxedValue(EvaluatorCommon.eval_bin_int_op(op, n1, n2))
        }
      | BoxedValue(_) => InvalidInput(3)
      | Indet(d2') => Indet(BinIntOp(op, d1', d2'))
      }
    | BoxedValue(_) => InvalidInput(4)
    | Indet(d1') =>
      switch (step_evaluate(d2)) {
      | Step(d2') => Step(BinIntOp(op, d1, d2'))
      | InvalidInput(msg) => InvalidInput(msg)
      | BoxedValue(d2')
      | Indet(d2') => Indet(BinIntOp(op, d1', d2'))
      }
    }
  | BinFloatOp(op, d1, d2) =>
    switch (step_evaluate(d1)) {
    | Step(d1') => Step(BinFloatOp(op, d1', d2))
    | InvalidInput(msg) => InvalidInput(msg)
    | BoxedValue(FloatLit(f1) as d1') =>
      switch (step_evaluate(d2)) {
      | Step(d2') => Step(BinFloatOp(op, d1, d2'))
      | InvalidInput(msg) => InvalidInput(msg)
      | BoxedValue(FloatLit(f2)) =>
        BoxedValue(EvaluatorCommon.eval_bin_float_op(op, f1, f2))
      | BoxedValue(_) => InvalidInput(8)
      | Indet(d2') => Indet(BinFloatOp(op, d1', d2'))
      }
    | BoxedValue(_) => InvalidInput(7)
    | Indet(d1') =>
      switch (step_evaluate(d2)) {
      | Step(d2') => Step(BinFloatOp(op, d1, d2'))
      | InvalidInput(msg) => InvalidInput(msg)
      | BoxedValue(d2')
      | Indet(d2') => Indet(BinFloatOp(op, d1', d2'))
      }
    }
  | Inj(ty, side, d1) =>
    switch (step_evaluate(d1)) {
    | Step(d1') => Step(Inj(ty, side, d1'))
    | InvalidInput(msg) => InvalidInput(msg)
    | BoxedValue(d1') => BoxedValue(Inj(ty, side, d1'))
    | Indet(d1') => Indet(Inj(ty, side, d1'))
    }
  | Pair(d1, d2) =>
    switch (step_evaluate(d1), step_evaluate(d2)) {
    | (Step(d1'), _) => Step(Pair(d1', d2))
    | (_, Step(d2')) => Step(Pair(d1, d2'))
    | (InvalidInput(msg), _)
    | (_, InvalidInput(msg)) => InvalidInput(msg)
    | (Indet(d1), Indet(d2))
    | (Indet(d1), BoxedValue(d2))
    | (BoxedValue(d1), Indet(d2)) => Indet(Pair(d1, d2))
    | (BoxedValue(d1), BoxedValue(d2)) => BoxedValue(Pair(d1, d2))
    }
  | Cons(d1, d2) =>
    switch (step_evaluate(d1), step_evaluate(d2)) {
    | (Step(d1'), _) => Step(Cons(d1', d2))
    | (_, Step(d2')) => Step(Cons(d1, d2'))
    | (InvalidInput(msg), _)
    | (_, InvalidInput(msg)) => InvalidInput(msg)
    | (Indet(d1), Indet(d2))
    | (Indet(d1), BoxedValue(d2))
    | (BoxedValue(d1), Indet(d2)) => Indet(Cons(d1, d2))
    | (BoxedValue(d1), BoxedValue(d2)) => BoxedValue(Cons(d1, d2))
    }
  | ConsistentCase(Case(d1, rules, n)) => evaluate_case(None, d1, rules, n)
  | InconsistentBranches(u, i, sigma, Case(d1, rules, n)) =>
    evaluate_case(Some((u, i, sigma)), d1, rules, n)
  | EmptyHole(_) => Indet(d)
  | NonEmptyHole(reason, u, i, sigma, d1) =>
    switch (step_evaluate(d1)) {
    | Step(d1') => Step(NonEmptyHole(reason, u, i, sigma, d1'))
    | InvalidInput(msg) => InvalidInput(msg)
    | BoxedValue(d1')
    | Indet(d1') => Indet(NonEmptyHole(reason, u, i, sigma, d1'))
    }
  | FreeVar(_) => Indet(d)
  | Keyword(_) => Indet(d)
  | InvalidText(_) => Indet(d)
  | Cast(d1, ty, ty') =>
    switch (step_evaluate(d1)) {
    | Step(d1') => Step(Cast(d1', ty, ty'))
    | InvalidInput(msg) => InvalidInput(msg)
    | BoxedValue(d1') as result =>
      switch (
        EvaluatorCommon.ground_cases_of(ty),
        EvaluatorCommon.ground_cases_of(ty'),
      ) {
      | (Hole, Hole) => result
      | (Ground, Ground) =>
        /* if two types are ground and consistent, then they are eq */
        result
      | (Ground, Hole) =>
        /* can't remove the cast or do anything else here, so we're done */
        BoxedValue(Cast(d1', ty, ty'))
      | (Hole, Ground) =>
        /* by canonical forms, d1' must be of the form d<ty'' -> ?> */
        switch (d1') {
        | Cast(d1'', ty'', Hole) =>
          if (HTyp.eq(ty'', ty')) {
            BoxedValue(d1'');
          } else {
            Indet(FailedCast(d1', ty, ty'));
          }
        | _ =>
          // TODO: can we omit this? or maybe call logging? JSUtil.log(DHExp.constructor_string(d1'));
          InvalidInput(6)
        }
      | (Hole, NotGroundOrHole(ty'_grounded)) =>
        /* ITExpand rule */
        Step(DHExp.Cast(Cast(d1', ty, ty'_grounded), ty'_grounded, ty'))
      | (NotGroundOrHole(ty_grounded), Hole) =>
        /* ITGround rule */
        Step(DHExp.Cast(Cast(d1', ty, ty_grounded), ty_grounded, ty'))
      | (Ground, NotGroundOrHole(_))
      | (NotGroundOrHole(_), Ground) =>
        /* can't do anything when casting between diseq, non-hole types */
        BoxedValue(Cast(d1', ty, ty'))
      | (NotGroundOrHole(_), NotGroundOrHole(_)) =>
        /* they might be eq in this case, so remove cast if so */
        if (HTyp.eq(ty, ty')) {
          result;
        } else {
          BoxedValue(Cast(d1', ty, ty'));
        }
      }
    | Indet(d1') as result =>
      switch (
        EvaluatorCommon.ground_cases_of(ty),
        EvaluatorCommon.ground_cases_of(ty'),
      ) {
      | (Hole, Hole) => result
      | (Ground, Ground) =>
        /* if two types are ground and consistent, then they are eq */
        result
      | (Ground, Hole) =>
        /* can't remove the cast or do anything else here, so we're done */
        Indet(Cast(d1', ty, ty'))
      | (Hole, Ground) =>
        switch (d1') {
        | Cast(d1'', ty'', Hole) =>
          if (HTyp.eq(ty'', ty')) {
            Indet(d1'');
          } else {
            Indet(FailedCast(d1', ty, ty'));
          }
        | _ => Indet(Cast(d1', ty, ty'))
        }
      | (Hole, NotGroundOrHole(ty'_grounded)) =>
        /* ITExpand rule */
        Step(DHExp.Cast(Cast(d1', ty, ty'_grounded), ty'_grounded, ty'))
      | (NotGroundOrHole(ty_grounded), Hole) =>
        /* ITGround rule */
        Step(DHExp.Cast(Cast(d1', ty, ty_grounded), ty_grounded, ty'))
      | (Ground, NotGroundOrHole(_))
      | (NotGroundOrHole(_), Ground) =>
        /* can't do anything when casting between diseq, non-hole types */
        Indet(Cast(d1', ty, ty'))
      | (NotGroundOrHole(_), NotGroundOrHole(_)) =>
        /* it might be eq in this case, so remove cast if so */
        if (HTyp.eq(ty, ty')) {
          result;
        } else {
          Indet(Cast(d1', ty, ty'));
        }
      }
    }
  | FailedCast(d1, ty, ty') =>
    switch (step_evaluate(d1)) {
    | Step(d1') => Step(FailedCast(d1', ty, ty'))
    | InvalidInput(msg) => InvalidInput(msg)
    | BoxedValue(d1')
    | Indet(d1') => Indet(FailedCast(d1', ty, ty'))
    }
  | InvalidOperation(d0, err) =>
    switch (step_evaluate(d0)) {
    | Step(d0') => Step(InvalidOperation(d0', err))
    | InvalidInput(msg) => InvalidInput(msg)

    | BoxedValue(d0')
    | Indet(d0') => Indet(InvalidOperation(d0', err))
    }
  }
and evaluate_case =
    (
      inconsistent_info,
      scrut: DHExp.t,
      rules: list(DHExp.rule),
      current_rule_index: int,
    )
    : step_status =>
  switch (step_evaluate(scrut)) {
  | Step(scrut') =>
    let case = DHExp.Case(scrut', rules, current_rule_index);
    switch (inconsistent_info) {
    | None => Step(ConsistentCase(case))
    | Some((u, i, sigma)) => Step(InconsistentBranches(u, i, sigma, case))
    };
  | InvalidInput(msg) => InvalidInput(msg)
  | BoxedValue(scrut)
  | Indet(scrut) =>
    switch (List.nth_opt(rules, current_rule_index)) {
    | None =>
      let case = DHExp.Case(scrut, rules, current_rule_index);
      switch (inconsistent_info) {
      | None => Indet(ConsistentCase(case))
      | Some((u, i, sigma)) =>
        Indet(InconsistentBranches(u, i, sigma, case))
      };
    | Some(Rule(dp, d)) =>
      switch (Elaborator_Exp.matches(dp, scrut)) {
      | Indet =>
        let case = DHExp.Case(scrut, rules, current_rule_index);
        switch (inconsistent_info) {
        | None => Indet(ConsistentCase(case))
        | Some((u, i, sigma)) =>
          Indet(InconsistentBranches(u, i, sigma, case))
        };
      | Matches(env) => Step(Elaborator_Exp.subst(env, d))
      | DoesNotMatch =>
        evaluate_case(inconsistent_info, scrut, rules, current_rule_index + 1)
      }
    }
  };
