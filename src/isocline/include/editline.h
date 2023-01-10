#pragma once
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "term.h"
#include "tty.h"
#include "env.h"
#include "stringbuf.h"
#include "history.h"
#include "completions.h"
#include "undo.h"
#include "highlight.h"

//-------------------------------------------------------------
// The editor state
//-------------------------------------------------------------



// editor state
typedef struct editor_s {
	stringbuf_t* input;        // current user input
	stringbuf_t* extra;        // extra displayed info (for completion menu etc)
	stringbuf_t* hint;         // hint displayed as part of the input
	stringbuf_t* hint_help;    // help for a hint.
	ssize_t       pos;          // current cursor position in the input
	ssize_t       cur_rows;     // current used rows to display our content (including extra content)
	ssize_t       cur_row;      // current row that has the cursor (0 based, relative to the prompt)
	ssize_t       termw;
	bool          modified;     // has a modification happened? (used for history navigation for example)
	bool          disable_undo; // temporarily disable auto undo (for history search)
	ssize_t       history_idx;  // current index in the history
	editstate_t* undo;         // undo buffer
	editstate_t* redo;         // redo buffer
	const char* prompt_text;  // text of the prompt before the prompt marker
	alloc_t* mem;          // allocator
	// caches
	attrbuf_t* attrs;        // reuse attribute buffers
	attrbuf_t* attrs_extra;
} editor_t;



