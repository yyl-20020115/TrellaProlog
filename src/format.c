#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "network.h"
#include "query.h"
#include "utf8.h"

static int format_integer(char *dst, pl_int_t v, int grouping, int sep, int decimals, int radix)
{
	char tmpbuf1[1024], tmpbuf2[1024];
	sprint_int(tmpbuf1, sizeof(tmpbuf1), v, radix);
	const char *src = tmpbuf1 + strlen(tmpbuf1) - 1;	// start from back
	char *dst2 = tmpbuf2;
	int i = 1, j = 1;

	while (src >= tmpbuf1) {
		*dst2++ = *src--;

		if (grouping && !decimals && !(i++ % grouping) && *src)
			*dst2++ = sep;

		if (decimals && (j++ == decimals)) {
			*dst2++ = '.';
			decimals = 0;
			i = 1;
		}
	}

	*dst2 = '\0';
	src = tmpbuf2 + (strlen(tmpbuf2) - 1);
	dst2 = dst;

	while (src >= tmpbuf2)
		*dst2++ = *src--;

	*dst2 = '\0';
	return dst2 - dst;
}

typedef struct {
	cell *p;
	pl_idx_t p_ctx;
	const char *srcbuf;
	const char *src;
	size_t srclen;
}
 list_reader_t;

static int get_next_char(query *q, list_reader_t *fmt)
{
	if (fmt->src) {
		int len = len_char_utf8(fmt->src);
		int ch = get_char_utf8(&fmt->src);
		fmt->srclen -= len;
		return ch;
	}

	fmt->p = fmt->p + 1;
	cell *head = deref(q, fmt->p, fmt->p_ctx);
	char ch;

	if (is_smallint(head))
		ch = get_smallint(head);
	else if (is_atom(head)) {
		const char *s = C_STR(q, head);
		ch = peek_char_utf8(s);
	} else
		return -1;

	fmt->p = fmt->p + fmt->p->nbr_cells;
	fmt->p = deref(q, fmt->p, fmt->p_ctx);
	fmt->p_ctx = q->latest_ctx;
	return ch;
}

static cell *get_next_cell(query *q, list_reader_t *fmt)
{
	if (fmt->src)
		return NULL;

	if (!is_list(fmt->p))
		return NULL;

	cell *head = fmt->p + 1;
	cell *tail = head + head->nbr_cells;
	head = deref(q, head, fmt->p_ctx);
	pl_idx_t save_ctx = q->latest_ctx;
	tail = deref(q, tail, fmt->p_ctx);
	fmt->p = tail;
	fmt->p_ctx = q->latest_ctx;
	q->latest_ctx = save_ctx;
	return head;
}

static bool is_more_data(query *q, list_reader_t *fmt)
{
	(void)q;

	if (fmt->src)
		return fmt->srclen;

	if (!fmt->p)
		return false;

	return is_list(fmt->p);
}

#define CHECK_BUF(len) {									\
    int n = (len) > 0 ? len : 1;                            \
	if (nbytes <= (unsigned)(1+n+1)) {    					\
		size_t save = dst - tmpbuf;							\
		bufsiz += len;										\
		tmpbuf = realloc(tmpbuf, bufsiz*=2);				\
		check_heap_error(tmpbuf);								\
		dst = tmpbuf + save;								\
		nbytes = bufsiz - save;								\
	}                                                       \
}

bool do_format(query *q, cell *str, pl_idx_t str_ctx, cell *p1, pl_idx_t p1_ctx, cell *p2, pl_idx_t p2_ctx)
{
	list_reader_t fmt1 = {0}, fmt2 = {0};
	list_reader_t save_fmt1 = {0}, save_fmt2 = {0};
	fmt1.p = p1;
	fmt1.p_ctx = p1_ctx;
	fmt1.srcbuf = is_atom(p1) ? C_STR(q, p1) : NULL;
	fmt1.srclen = is_atom(p1) ? C_STRLEN(q, p1) : 0;
	fmt1.src = fmt1.srcbuf;
	fmt2.p = p2;
	fmt2.p_ctx = p2_ctx;

	size_t bufsiz = 1024*8;
	char *tmpbuf = malloc(bufsiz);
	check_heap_error(tmpbuf);
	char *dst = tmpbuf;
	*dst = '\0';
	size_t nbytes = bufsiz;
	bool redo = false, start_of_line = true;
	int tab_at = 1, tabs = 0, diff = 0, last_at = 0, tab_char = ' ';
	save_fmt1 = fmt1;
	save_fmt2 = fmt2;

	while (is_more_data(q, &fmt1)) {
		int argval = 0, noargval = 1;
		int pos = dst - tmpbuf + 1;
        list_reader_t tmp_fmt1 = fmt1, tmp_fmt2 = fmt2;

		int ch = get_next_char(q, &fmt1);

		if (ch != '~') {
            CHECK_BUF(MAX_BYTES_PER_CODEPOINT);
			dst += put_char_utf8(dst, ch);
			start_of_line = ch == '\n';
			continue;
		}

		ch = get_next_char(q, &fmt1);

		if (ch == '*') {
			cell *c = get_next_cell(q, &fmt2);
			noargval = 0;

			if (!c || !is_integer(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "integer");
			}

			argval = get_smallint(c);
			ch = get_next_char(q, &fmt1);
		} else if (ch == '`') {
			ch = get_next_char(q, &fmt1);
			argval = ch;
			ch = get_next_char(q, &fmt1);
		} else {
			while (isdigit(ch)) {
				noargval = 0;
				argval *= 10;
				argval += ch - '0';
				ch = get_next_char(q, &fmt1);
				continue;
			}
		}

		CHECK_BUF(argval);

		if (ch == 'n') {
			while (argval-- > 1)
				*dst++ = '\n';

			*dst++ = '\n';
			start_of_line = true;
			continue;
		}

		if (ch == 'N') {
			if (!start_of_line) {
				start_of_line = true;
                CHECK_BUF(1);
				*dst++ = '\n';
			}

			continue;
		}

		if (ch == 't') {
            if (!redo && !tabs) {
                save_fmt1 = tmp_fmt1;
                save_fmt2 = tmp_fmt2;
                tab_at = pos;
                tabs++;
            } else if (!redo) {
                tabs++;
            } else if (redo) {
				tab_char = argval ? argval : ' ';

                for (int i = 0; i < diff; i++) {
                    CHECK_BUF(MAX_BYTES_PER_CODEPOINT);
					dst += put_char_utf8(dst, tab_char);
                }
            }

			continue;
		}

		if (ch == '|') {
			int at = last_at = argval ? argval : pos;

            if (!argval)
                last_at -= 1;

            if (!tabs)
                continue;

			if (!redo) {
                if (!tabs) {
                    tab_at = pos;
                    dst = tmpbuf + tab_at - 1;
                    diff = (at - pos) + 1;

                    for (int i = 0; i < diff; i++) {
                        CHECK_BUF(MAX_BYTES_PER_CODEPOINT);
						dst += put_char_utf8(dst, tab_char);
                    }
                } else {
                    fmt1 = save_fmt1;
                    fmt2 = save_fmt2;
                    dst = tmpbuf + tab_at - 1;
                    diff = ((at - pos) + 1) / tabs;
                }
			} else {
                tabs = 0;
            }

			redo = !redo;
			continue;
		}

		if (ch == '+') {
            if (!tabs)
                continue;

			if (!redo) {
				int at = last_at = argval ? (last_at+argval) : pos;

                if (!tabs) {
                    tab_at = pos;
                    dst = tmpbuf + tab_at - 1;
                    diff = (at - pos) + 1;

                    for (int i = 0; i < diff; i++) {
                        CHECK_BUF(MAX_BYTES_PER_CODEPOINT);
						dst += put_char_utf8(dst, tab_char);
                    }
                } else {
                    fmt1 = save_fmt1;
                    fmt2 = save_fmt2;
                    dst = tmpbuf + tab_at - 1;
                    diff = ((at - pos) + 1) / tabs;
                }
			} else {
                tabs = 0;
            }

			redo = !redo;
			continue;
		}

		if (ch == '~') {
            CHECK_BUF(1);
			*dst++ = '~';
			start_of_line = false;
			continue;
		}

		if (!p2 || !is_list(p2)) {
			cell tmp;
			make_atom(&tmp, g_nil_s);
			return throw_error(q, &tmp, q->st.curr_frame, "domain_error", "missing_args");
		}

		cell *c = get_next_cell(q, &fmt2);

		if (!c)
			return throw_error(q, p2, p2_ctx, "domain_error", "missing_args");

		if (ch == 'i')
			continue;

        pl_idx_t c_ctx = q->latest_ctx;
		start_of_line = false;
		size_t len = 0;

		if ((ch == 'a') && !is_atom(c)) {
			free(tmpbuf);
			return throw_error(q, c, q->st.curr_frame, "type_error", "atom");
		}

		switch(ch) {
		case 's':
			if (is_string(c)) {
				len = MAX_OF(argval, (int)C_STRLEN(q, c));
				CHECK_BUF(len);
				memcpy(dst, C_STR(q, c), len);
			} else {
				list_reader_t fmt3 = {0};
				fmt3.p = c;
				fmt3.p_ctx = c_ctx;
				int cnt = 0;

				while (is_more_data(q, &fmt3)) {
					int ch = get_next_char(q, &fmt3);
					CHECK_BUF(MAX_BYTES_PER_CODEPOINT);
					dst += put_char_utf8(dst, ch);
					cnt++;

					if (cnt == argval)
						break;
				}

				len = 0;
			}

			break;

		case 'c':
			if (!is_integer(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "integer");
			}

			while (argval-- > 1) {
				CHECK_BUF(MAX_BYTES_PER_CODEPOINT);
				dst += put_char_utf8(dst, (int)get_smallint(c));
			}

			CHECK_BUF(MAX_BYTES_PER_CODEPOINT);
			dst += put_char_utf8(dst, (int)get_smallint(c));
			len = 0;
			break;

		case 'e':
		case 'E':
			if (!is_float(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "float");
			}

			len = 40;
			CHECK_BUF(len);

			if (argval) {
				if (ch == 'e')
					len = sprintf(dst, "%.*e", argval, get_float(c));
				else
					len = sprintf(dst, "%.*E", argval, get_float(c));
			} else {
				if (ch == 'e')
					len = sprintf(dst, "%e", get_float(c));
				else
					len = sprintf(dst, "%E", get_float(c));
			}

			break;

		case 'g':
		case 'G':
			if (!is_float(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "float");
			}

			len = 40;
			CHECK_BUF(len);

			if (argval) {
				if (ch == 'g')
					len = sprintf(dst, "%.*g", argval, get_float(c));
				else
					len = sprintf(dst, "%.*G", argval, get_float(c));
			} else {
				if (ch == 'g')
					len = sprintf(dst, "%g", get_float(c));
				else
					len = sprintf(dst, "%G", get_float(c));
			}

			break;

		case 'f':
			if (!is_float(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "float");
			}

			len = 40;
			CHECK_BUF(len);

			if (argval)
				len = sprintf(dst, "%.*f", argval, get_float(c));
			else
				len = sprintf(dst, "%f", get_float(c));

			break;

		case 'I':
			if (!is_integer(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "integer");
			}

			len = 40;
			CHECK_BUF(len);
			len = format_integer(dst, get_smallint(c), noargval?3:argval, '_', 0, 10);
			break;

		case 'd':
			if (!is_integer(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "integer");
			}

			len = 40;
			CHECK_BUF(len);
			len = format_integer(dst, get_smallint(c), 0, ',', noargval?0:argval, 10);
			break;

		case 'D':
			if (!is_integer(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "integer");
			}

			len = 40;
			CHECK_BUF(len);
			len = format_integer(dst, get_smallint(c), 3, ',', noargval?0:argval, 10);
			break;

		case 'r':
			if (!noargval && ((argval < 2) || (argval > 36))) {
				free(tmpbuf);
				return throw_error(q, p1, p1_ctx, "domain_error", "radix_invalid");
			}

			if (!is_integer(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "integer");
			}

			len = 40;
			CHECK_BUF(len);
			len = format_integer(dst, get_smallint(c), 0, ',', 0, !argval?8:argval);
			break;

		case 'R':
			if (!noargval && ((argval < 2) || (argval > 36))) {
				free(tmpbuf);
				return throw_error(q, p1, p1_ctx, "domain_error", "radix_invalid");
			}

			if (!is_integer(c)) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "type_error", "integer");
			}

			len = 40;
			CHECK_BUF(len);
			len = format_integer(dst, get_smallint(c), 0, ',', 0, !argval?-8:-argval);
			break;

		case 'k':
		case 'q':
		case 'w':
		case 'a':
        {
			int saveq = q->quoted;
			bool canonical = false, quoted = false;
			q->numbervars = true;

			if (ch == 'k') {
				canonical = true;
			} else if (ch == 'q') {
				quoted = true;
			}

			if (quoted)
				q->quoted = 1;

			if (is_string(c) && !q->quoted)
				q->quoted = -1;

			if (argval)
				q->max_depth = argval;

			if (canonical) {
				q->ignore_ops = true;
				q->quoted = 1;
			}

			len = print_term_to_buf(q, NULL, 0, c, c_ctx, 1, false, 0);

			if (q->cycle_error) {
				free(tmpbuf);
				return throw_error(q, c, q->st.curr_frame, "resource_error", "cyclic");
            }

			CHECK_BUF(len*2);
			len = print_term_to_buf(q, dst, len+1, c, c_ctx, 1, false, 0);
			clear_write_options(q);
			q->quoted = saveq;
            break;
        }

        default:
			free(tmpbuf);
			return throw_error(q, c, q->st.curr_frame, "existence_error", "format_character");
		}

		dst += len;
		nbytes -= len;
	}

	*dst = '\0';
	size_t len = dst - tmpbuf;

	if (str == NULL) {
		int n = q->st.m->pl->current_output;
		stream *str = &q->pl->streams[n];
		net_write(tmpbuf, len, str);
	} else if (is_structure(str)
		&& ((CMP_STR_TO_CSTR(q, str, "atom")
		&& CMP_STR_TO_CSTR(q, str, "chars")
		&& CMP_STR_TO_CSTR(q, str, "string"))
		|| (str->arity > 1) || !is_var(str+1))) {
		free(tmpbuf);
		return throw_error(q, str, str_ctx, "type_error", "structure");
	} else if (is_structure(str) && !CMP_STR_TO_CSTR(q, str, "atom")) {
		cell *c = deref(q, str+1, str_ctx);
		cell tmp;
		check_heap_error2(make_cstringn(&tmp, tmpbuf, len), free(tmpbuf));
		set_var(q, c, q->latest_ctx, &tmp, q->st.curr_frame);
		unshare_cell(&tmp);
	} else if (is_structure(str)) {
		cell *c = deref(q, str+1, str_ctx);
		cell tmp;

		if (strlen(tmpbuf))
			check_heap_error2(make_stringn(&tmp, tmpbuf, len), free(tmpbuf));
		else
			make_atom(&tmp, g_nil_s);

		set_var(q, c, q->latest_ctx, &tmp, q->st.curr_frame);
		unshare_cell(&tmp);
	} else if (is_stream(str)) {
		int n = get_stream(q, str);
		stream *str = &q->pl->streams[n];
		const char *tmpsrc = tmpbuf;

		while (len) {
			size_t nbytes = net_write(tmpsrc, len, str);

			if (!nbytes) {
				if (feof(str->fp) || ferror(str->fp)) {
					free(tmpbuf);
					fprintf(stdout, "Error: end of file on write\n");
					return false;
				}
			}

			clearerr(str->fp);
			len -= nbytes;
			tmpsrc += nbytes;
		}
	} else {
		free(tmpbuf);
		return throw_error(q, str, str_ctx, "domain_error", "stream_or_alias");
	}

	free(tmpbuf);
	return true;
}

