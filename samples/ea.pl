% Original code from https://rosettacode.org/wiki/Evolutionary_algorithm#Prolog

:- initialization(main).

main :-
    solve('METHINKS IT IS LIKE A WEASEL').

solve(TargetAtom) :-
    atom_codes(TargetAtom, Target),
    length(Target, Len),
    random_text(Len, Start),
    evolve(0, 5, Target, Start).    % evolution 0 and 5% probability for a mutation

random_text(0, []).                 % generate some random text (fixed length)
random_text(Len, [H|T]) :-
    succ(L, Len),
    random_alpha(H),
    random_text(L, T).

random_alpha(Ch) :-                 % generate a single random character
    P is rand mod 27,
    (   P = 0
    ->  Ch is 32
    ;   Ch is P+64
    ).

evolve(Evolution, Probability, Target, mutation(Score, Value)) :-
    atom_codes(Val, Value),
    format("evolution=~w score=~w value=~q~n", [Evolution, Score, Val]),
    (   Score = 0
    ->  true
    ;   evolve(Evolution, Probability, Target, Value)
    ).
evolve(Evolution, Probability, Target, Start) :-
    findall(mutation(Score, M),     % generate 80 mutations, select the best
        (   between(1, 80, _),
            mutate(Probability, Start, M),
            score(M, Score, Target)
        ),
        Mutations
    ),
    sort(Mutations, [Best|_]),
    succ(Evolution, Evo),
    evolve(Evo, Probability, Target, Best).

mutate(_, [], []).                  % mutate(Probability, Input, Output)
mutate(Probability, [H|Txt], [H|Mut]) :-
    R is rand mod 100,
    R > Probability,
    !,
    mutate(Probability, Txt, Mut).
mutate(Probability, [_|Txt], [M|Mut]) :-
    random_alpha(M),
    mutate(Probability, Txt, Mut).

score(Txt, Score, Target) :-
    score(Target, Txt, 0, Score).

score([], [], Score, Score).        % score a generated mutation (count diffs)
score([Ht|Tt], [Ht|Tp], C, Score) :-
    !,
    score(Tt, Tp, C, Score).
score([_|Tt], [_|Tp], C, Score) :-
    succ(C, N),
    score(Tt, Tp, N, Score).
