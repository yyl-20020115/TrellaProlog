:- use_module(library(lists)).

% sudoku solver, illustrating prolog programming/backtracking, not optimized for performance


%transform a puzzle from usual Lines notation into Columns and Blocks notation
transform(
	[	% Lines
	[AA, AB, AC, AD, AE, AF, AG, AH, AI],
	[BA, BB, BC, BD, BE, BF, BG, BH, BI],
	[CA, CB, CC, CD, CE, CF, CG, CH, CI],
	[DA, DB, DC, DD, DE, DF, DG, DH, DI],
	[EA, EB, EC, ED, EE, EF, EG, EH, EI],
	[FA, FB, FC, FD, FE, FF, FG, FH, FI],
	[GA, GB, GC, GD, GE, GF, GG, GH, GI],
	[HA, HB, HC, HD, HE, HF, HG, HH, HI],
	[IA, IB, IC, ID, IE, IF, IG, IH, II]]
	,[	% Columns
	[AA, BA, CA, DA, EA, FA, GA, HA, IA],
	[AB, BB, CB, DB, EB, FB, GB, HB, IB],
	[AC, BC, CC, DC, EC, FC, GC, HC, IC],
	[AD, BD, CD, DD, ED, FD, GD, HD, ID],
	[AE, BE, CE, DE, EE, FE, GE, HE, IE],
	[AF, BF, CF, DF, EF, FF, GF, HF, IF],
	[AG, BG, CG, DG, EG, FG, GG, HG, IG],
	[AH, BH, CH, DH, EH, FH, GH, HH, IH],
	[AI, BI, CI, DI, EI, FI, GI, HI, II]]
	,[	% Blocks
	[AA, AB, AC, BA, BB, BC, CA, CB, CC],
	[AD, AE, AF, BD, BE, BF, CD, CE, CF],
	[AG, AH, AI, BG, BH, BI, CG, CH, CI],
	[DA, DB, DC, EA, EB, EC, FA, FB, FC],
	[DD, DE, DF, ED, EE, EF, FD, FE, FF],
	[DG, DH, DI, EG, EH, EI, FG, FH, FI],
	[GA, GB, GC, HA, HB, HC, IA, IB, IC],
	[GD, GE, GF, HD, HE, HF, ID, IE, IF],
	[GG, GH, GI, HG, HH, HI, IG, IH, II]]).

% fast member/2 which ignores variables, for checking only
memberchk_nonvar(Element, [E|_]) :- nonvar(E), E=Element, !.
memberchk_nonvar(Element, [_|Rest]) :-
	memberchk_nonvar(Element, Rest).

no_dupes([H|T]) :- nonvar(H), \+ memberchk_nonvar(H,T), no_dupes(T).
no_dupes([H|T]) :- var(H), no_dupes(T).
no_dupes([]).

sudoku(Puzzle) :-
	transform(Puzzle, Columns, Blocks),
	sudoku([], Puzzle, Columns, Blocks).

sudoku(LinesSoFar, [Line|Rest], Columns, Blocks) :-
	generate_line(LinesSoFar, [], Line, Columns, Blocks),
	append(LinesSoFar, [Line], NewLinesSoFar),
	sudoku(NewLinesSoFar, Rest, Columns, Blocks).
sudoku(_, [], _, _).


generate_line(LinesSoFar, ElementsSoFar, [H|T], Columns, Blocks) :-
	member(H, [1,2,3,4,5,6,7,8,9]),
	append(ElementsSoFar,[H|T],Line),

	no_dupes(Line),

	length(ElementsSoFar, Y),
	nth0(Y, Columns, Column),
	no_dupes(Column),

	length(LinesSoFar, X),
	B is ((X mod 3) + 3 * ((Y) mod 3)),
	nth0(B, Blocks, Block),
	no_dupes(Block),

	append(ElementsSoFar,[H],NewElementsSoFar),
	generate_line(LinesSoFar, NewElementsSoFar, T, Columns, Blocks).
generate_line(_, _, [], _, _).

pretty_print([A,B,C|T]) :-
	pretty_print_line(A),
	pretty_print_line(B),
	pretty_print_line(C),
	( T \= []
	->	write('------+-------+-------'),nl,
		pretty_print(T)
	;	nl
	).

pretty_print_line([A,B,C|T]) :-
	write(A),write(' '),
	write(B),write(' '),
	write(C),
	( T \= []
	->	write(' | '),
		pretty_print_line(T)
	;	nl
	).


test1 :-
	L = [
	     [_,6,_,1,_,4,_,5,_],
	     [_,_,8,3,_,5,6,_,_],
	     [2,_,_,_,_,_,_,_,1],
	     [8,_,_,4,_,7,_,_,6],
	     [_,_,6,_,_,_,3,_,_],
	     [7,_,_,9,_,1,_,_,4],
	     [5,_,_,_,_,_,_,_,2],
	     [_,_,7,2,_,6,9,_,_],
	     [_,4,_,5,_,8,_,7,_]],
	sudoku(L),
	pretty_print(L).

%Fiendish puzzel April 21,2005 Times London
test2 :-
	L = [
	     [_,_,4 ,_,_,3, _,7,_],
	     [_,8,_ ,_,7,_, _,_,_],
	     [_,7,_ ,_,_,8, 2,_,5],
	     [4,_,_ ,_,_,_, 3,1,_],
	     [9,_,_ ,_,_,_, _,_,8],
	     [_,1,5 ,_,_,_, _,_,4],
	     [1,_,6 ,9,_,_, _,3,_],
	     [_,_,_ ,_,2,_, _,6,_],
	     [_,2,_ ,4,_,_, 5,_,_]],
	sudoku(L),
	pretty_print(L).


%This is supposed to be hard.
test3 :-
	L=
	[
	 [_,4,3,_,8,_,2,5,_],
	 [6,_,_,_,_,_,_,_,_],
	 [_,_,_,_,_,1,_,9,4],
	 [9,_,_,_,_,4,_,7,_],
	 [_,_,_,6,_,8,_,_,_],
	 [_,1,_,2,_,_,_,_,3],
	 [8,2,_,5,_,_,_,_,_],
	 [_,_,_,_,_,_,_,_,5],
	 [_,3,4,_,9,_,7,1,_]
	],
	sudoku(L),
	pretty_print(L).

test :- test1, !, test2, !, test3, !.
