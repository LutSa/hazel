open Sexplib.Std;

[@deriving sexp]
type t =
  | Tag(string)
  | TagHole(MetaVar.t);

let compare = compare;

let eq = (t1: t, t2: t): bool => t1 == t2;

let new_TagHole = (u_gen: MetaVarGen.t): (t, MetaVarGen.t) => {
  let (u, u_gen) = u_gen |> MetaVarGen.next;
  (TagHole(u), u_gen);
};

let is_majuscule_letter = (c: char): bool => {
  let code = Char.code(c);
  Char.code('A') <= code && code <= Char.code('Z');
};

let is_minuscule_letter = (c: char): bool => {
  let code = Char.code(c);
  Char.code('a') <= code && code <= Char.code('z');
};

let is_numeric_digit = (c: char): bool => {
  let code = Char.code(c);
  Char.code('0') <= code && code <= Char.code('9');
};

let is_punctuation: char => bool =
  fun
  | '_'
  | '\'' => true
  | _ => false;

let is_tag_char = (c: char): bool => {
  is_majuscule_letter(c)
  || is_minuscule_letter(c)
  || is_numeric_digit(c)
  || is_punctuation(c);
};

let is_tag_name = (str: string): bool => {
  switch (str |> String.to_seq |> List.of_seq) {
  | [] => false
  | [c] => is_majuscule_letter(c)
  | [c, ...cs] => is_majuscule_letter(c) && List.for_all(is_tag_char, cs)
  };
};

let of_text = (text: string): option(t) =>
  if (is_tag_name(text)) {
    Some(Tag(text));
  } else {
    None;
  };

let is_complete: t => bool =
  fun
  | Tag(_) => true
  | TagHole(_) => false;