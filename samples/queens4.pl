% Copyright (C) 1988,1989 Herve' Touati,Aquarius Project,UC Berkeley

% the queens on a chessboard problem (queens) for 4x4 board

test :- doit(4,_).

size(4).
int(1).
int(2).
int(3).
int(4).

doit(Size,Soln) :-
    get_solutions(Size,Soln),
    inform(Soln),
    fail.
doit(_,_).

doitq(Size,Soln) :-
    get_solutions(Size,Soln),
    fail.
doitq(_,_).

get_solutions(Board_size,Soln) :-
    solve(Board_size,[],Soln).

% newsquare generates legal positions for next queen

newsquare([],square(1,X)) :-
    int(X).
newsquare([square(I,J)|Rest],square(X,Y)) :-
    X is I + 1,
    int(Y),
    \+ threatened(I,J,X,Y),
    safe(X,Y,Rest).

% safe checks whether square(X,Y) is threatened by any
% existing queens

safe(_,_,[]).
safe(X,Y,[square(I,J)|L]) :-
    \+ threatened(I,J,X,Y),
    safe(X,Y,L).

% threatened checks whether squares (I,J) and (X,Y)
% threaten each other

threatened(I,_,X,_) :-
    I = X.
threatened(_,J,_,Y) :-
    J = Y.
threatened(I,J,X,Y) :-
    U is I - J,
    V is X - Y,
    U = V.
threatened(I,J,X,Y) :-
    U is I + J,
    V is X + Y,
    U = V.

% solve accumulates the positions of occupied squares

solve(Bs,[square(Bs,Y)|L],[square(Bs,Y)|L]) :-
    size(Bs).
solve(Bs,Initial,Final) :-
    newsquare(Initial,Next),
    solve(Bs,[Next|Initial],Final).

inform([]) :- nl,nl.
inform([M|L]) :- write(M),nl,inform(L).
