:- module(dif, [dif/2]).

% This is borrowed from Scryer-Prolog

:- use_module(library(atts)).
:- use_module(library(dcgs)).
:- use_module(library(lists), [append/3]).

:- attribute dif/1.

put_dif_att(Var, X, Y) :-
    (   (get_atts(Var, +dif(Z)), nonvar(Z)) ->
	    sort([X \== Y | Z], NewZ),
	    put_atts(Var, +dif(NewZ))
    ;   put_atts(Var, +dif([X \== Y]))
    ).

dif_set_variables([], _, _).
dif_set_variables([Var|Vars], X, Y) :-
    put_dif_att(Var, X, Y),
    dif_set_variables(Vars, X, Y).

append_goals([], _).
append_goals([Var|Vars], Goals) :-
    (   get_atts(Var, +dif(VarGoals)) ->
	    append(Goals, VarGoals, NewGoals0),
	    sort(NewGoals0, NewGoals)
    ;   NewGoals = Goals
    ),
    put_atts(Var, +dif(NewGoals)),
    append_goals(Vars, Goals).

verify_attributes(Var, Value, Goals) :-
    (   get_atts(Var, +dif(Goals)) ->
	    term_variables(Value, ValueVars),
	    append_goals(ValueVars, Goals)
    ;   Goals = []
    ).

% Probably the world's worst dif/2 implementation. I'm open to
% suggestions for improvement.

dif(X, Y) :-
    X \== Y,
    (   X \= Y -> true
    ;   (   term_variables(X, XVars),
            term_variables(Y, YVars),
            dif_set_variables(XVars, X, Y),
            dif_set_variables(YVars, X, Y)
        )
    ).

gather_dif_goals([]) --> [].
gather_dif_goals([(X \== Y) | Goals]) -->
    [dif(X, Y)],
    gather_dif_goals(Goals).

attribute_goals(X) -->
    { get_atts(X, +dif(Goals)) },
    gather_dif_goals(Goals),
    { put_atts(X, -dif(_)) }.
