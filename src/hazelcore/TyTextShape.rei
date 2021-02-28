[@deriving sexp]
type t =
  | Int
  | Float
  | Bool
  | ExpandingKeyword(ExpandingKeyword.t)
  | TyVar(TyId.t);

let of_builtIn_opt: TyId.t => option(t);

let of_tyid: TyId.t => option(t);
