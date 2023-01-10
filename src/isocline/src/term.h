/* ----------------------------------------------------------------------------
  Copyright (c) 2021, Daan Leijen
  This is free software; you can redistribute it and/or modify it
  under the terms of the MIT License. A copy of the license can be
  found in the "LICENSE" file at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef IC_TERM_H
#define IC_TERM_H

#include "common.h"
#include "tty.h"
#include "stringbuf.h"
#include "attr.h"
#ifndef IC_CSI
#define IC_CSI      "\x1B["
#endif
#ifdef _WIN32
#include <wincontypes.h>
#endif
//struct term_s;

// color support; colors are auto mapped smaller palettes if needed. (see `term_color.c`)
typedef enum palette_e {
	MONOCHROME,  // no color
	ANSI8,       // only basic 8 ANSI color     (ESC[<idx>m, idx: 30-37, +10 for background)
	ANSI16,      // basic + bright ANSI colors  (ESC[<idx>m, idx: 30-37, 90-97, +10 for background)
	ANSI256,     // ANSI 256 color palette      (ESC[38;5;<idx>m, idx: 0-15 standard color, 16-231 6x6x6 rbg colors, 232-255 gray shades)
	ANSIRGB      // direct rgb colors supported (ESC[38;2;<r>;<g>;<b>m)
} palette_t;

typedef enum buffer_mode_e {
	UNBUFFERED,
	LINEBUFFERED,
	BUFFERED,
} buffer_mode_t;

// The terminal screen
struct term_s {
	int           fd_out;             // output handle
	ssize_t       width;              // screen column width
	ssize_t       height;             // screen row height
	ssize_t       raw_enabled;        // is raw mode active? counted by start/end pairs
	bool          nocolor;            // show colors?
	bool          silent;             // enable beep?
	bool          is_utf8;            // utf-8 output? determined by the tty
	attr_t   attr;               // current text attributes
	palette_t     palette;            // color support
	buffer_mode_t bufmode;            // buffer mode
	stringbuf_t* buf;                // buffer for buffered output
	tty_t* tty;                // used on posix to get the cursor position
	alloc_t* mem;                // allocator
#ifdef _WIN32
	HANDLE        hcon;               // output console handler
	WORD          hcon_default_attr;  // default text attributes
	WORD          hcon_orig_attr;     // original text attributes
	DWORD         hcon_orig_mode;     // original console mode
	DWORD         hcon_mode;          // used console mode
	UINT          hcon_orig_cp;       // original console code-page (locale)
	COORD         hcon_save_cursor;   // saved cursor position (for escape sequence emulation)
#endif
};


typedef struct term_s term_t;

// Primitives
ic_private term_t* term_new(alloc_t* mem, tty_t* tty, bool nocolor, bool silent, int fd_out);
ic_private void term_free(term_t* term);

ic_private bool term_is_interactive(const term_t* term);
ic_private void term_start_raw(term_t* term);
ic_private void term_end_raw(term_t* term, bool force);

ic_private bool term_enable_beep(term_t* term, bool enable);
ic_private bool term_enable_color(term_t* term, bool enable);

ic_private void term_flush(term_t* term);
ic_private buffer_mode_t term_set_buffer_mode(term_t* term, buffer_mode_t mode);

ic_private void term_write_n(term_t* term, const char* s, ssize_t n);
ic_private void term_write(term_t* term, const char* s);
ic_private void term_writeln(term_t* term, const char* s);
ic_private void term_write_char(term_t* term, char c);

ic_private void term_write_repeat(term_t* term, const char* s, ssize_t count );
ic_private void term_beep(term_t* term);

ic_private bool term_update_dim(term_t* term);

ic_private ssize_t term_get_width(term_t* term);
ic_private ssize_t term_get_height(term_t* term);
ic_private int  term_get_color_bits(term_t* term);

// Helpers
ic_private void term_writef(term_t* term, const char* fmt, ...);
ic_private void term_vwritef(term_t* term, const char* fmt, va_list args);

ic_private void term_left(term_t* term, ssize_t n);
ic_private void term_right(term_t* term, ssize_t n);
ic_private void term_up(term_t* term, ssize_t n);
ic_private void term_down(term_t* term, ssize_t n);
ic_private void term_start_of_line(term_t* term );
ic_private void term_clear_line(term_t* term);
ic_private void term_clear_to_end_of_line(term_t* term);
// ic_private void term_clear_lines_to_end(term_t* term);


ic_private void term_attr_reset(term_t* term);
ic_private void term_underline(term_t* term, bool on);
ic_private void term_reverse(term_t* term, bool on);
ic_private void term_bold(term_t* term, bool on);
ic_private void term_italic(term_t* term, bool on);

ic_private void term_color(term_t* term, ic_color_t color);
ic_private void term_bgcolor(term_t* term, ic_color_t color);

// Formatted output

ic_private attr_t term_get_attr( const term_t* term );
ic_private void   term_set_attr( term_t* term, attr_t attr );
ic_private void   term_write_formatted( term_t* term, const char* s, const attr_t* attrs );
ic_private void   term_write_formatted_n( term_t* term, const char* s, const attr_t* attrs, ssize_t n );

ic_private ic_color_t color_from_ansi256(ssize_t i);

#endif // IC_TERM_H
