/*
 * Copyright (c) 2018 annidy, Inc. All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stuff/bool.h"
#include "stuff/ofile.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"

char *progname = NULL;

struct flags
{
	enum bool treat_as_data;
	enum bool print_offsets;
	char *offset_format;
	enum bool all_sections;
	uint32_t minimum_length;
};

struct symbol {
    char *name;
    char *indr_name;
    struct nlist_64 nl;
	uint32_t nl_addr;
};

static void usage(
	void);
static void ofile_processor(
	struct ofile *ofile,
	char *arch_name,
	void *cookie);
static struct symbol *select_symbols(
    struct ofile *ofile,
    struct symtab_command *st,
    uint32_t *nsymbols);
static void make_symbol_32(
    struct symbol *symbol,
    struct nlist *nl);
static void make_symbol_64(
    struct symbol *symbol,
    struct nlist_64 *nl);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

int main(
	int argc,
	char **argv,
	char **envp)
{
	struct flags flags;
	int i;
	uint32_t j, nfiles;
	char *endp;
	struct arch_flag *arch_flags;
	uint32_t narch_flags;
	enum bool all_archs, rest_args_files, use_member_syntax;
	struct stat stat_buf;

	progname = argv[0];

	nfiles = 0;
	arch_flags = NULL;
	narch_flags = 0;
	all_archs = FALSE;

	flags.treat_as_data = FALSE;
	flags.print_offsets = FALSE;
	flags.offset_format = NULL;
	flags.all_sections = FALSE;
	flags.minimum_length = 4;

	rest_args_files = FALSE;
	for (i = 1; i < argc; i++)
	{
		if (rest_args_files == FALSE && argv[i][0] == '-')
		{
			if (argv[i][1] == '\0')
				flags.treat_as_data = TRUE;
			else if (strcmp(argv[i], "--") == 0)
				rest_args_files = TRUE;
			else if (strcmp(argv[i], "-arch") == 0)
			{
				if (i + 1 == argc)
				{
					error("missing argument(s) to %s option", argv[i]);
					usage();
				}
				if (strcmp("all", argv[i + 1]) == 0)
				{
					all_archs = TRUE;
				}
				else
				{
					arch_flags = reallocate(arch_flags,
											(narch_flags + 1) * sizeof(struct arch_flag));
					if (get_arch_from_flag(argv[i + 1],
										   arch_flags + narch_flags) == 0)
					{
						error("unknown architecture specification flag: "
							  "%s %s",
							  argv[i], argv[i + 1]);
						arch_usage();
						usage();
					}
					narch_flags++;
				}
				i++;
			}
		}
		else
		{
			nfiles++;
		}
	}

	/*
	 * Process the file or stdin if there are no files.
	 */
	rest_args_files = FALSE;
	if (nfiles != 0)
	{
		for (i = 1; i < argc; i++)
		{
			if (argv[i][0] != '-')
			{
			/*
			 * If there's a filename that's an exact match then use
			 * that, else fall back to the member syntax.
			 */
					if (stat(argv[i], &stat_buf) == 0)
						use_member_syntax = FALSE;
					else
						use_member_syntax = TRUE;
					ofile_process(argv[i], arch_flags, narch_flags,
								  all_archs, TRUE, TRUE, use_member_syntax,
								  ofile_processor, &flags);
			}
			else if (strcmp(argv[i], "-arch") == 0)
				i++;
			else if (strcmp(argv[i], "--") == 0)
				rest_args_files = TRUE;
		}
	}

	if (errors == 0)
		return (EXIT_SUCCESS);
	else
		return (EXIT_FAILURE);
}

/*
 * usage() prints the current usage message and exits indicating failure.
 */
static void
usage(
	void)
{
	fprintf(stderr, "Usage: %s [-] "
					"[[-arch <arch_flag>] ...] [--] [file ...]\n",
			progname);
	exit(EXIT_FAILURE);
}

/*
 * select_symbols returns an allocated array of symbol structs as the symbols
 * that are to be printed based on the flags.  The number of symbols in the
 * array returned in returned indirectly through nsymbols.
 */
static
struct symbol *
select_symbols(
struct ofile *ofile,
struct symtab_command *st,
uint32_t *nsymbols)
{
    uint32_t i, flags, nest;
    struct nlist *all_symbols;
    struct nlist_64 *all_symbols64;
    struct symbol *selected_symbols, symbol;

    enum bool found;

	if(ofile->mh != NULL){
	    all_symbols = (struct nlist *)(ofile->object_addr + st->symoff);
	    all_symbols64 = NULL;
	}
	else{
	    all_symbols = NULL;
	    all_symbols64 = (struct nlist_64 *)(ofile->object_addr + st->symoff);
	}
	selected_symbols = allocate(sizeof(struct symbol) * st->nsyms);
	*nsymbols = 0;

	if(ofile->object_byte_sex != get_host_byte_sex()){
	    if(ofile->mh != NULL)
		swap_nlist(all_symbols, st->nsyms, get_host_byte_sex());
	    else
		swap_nlist_64(all_symbols64, st->nsyms, get_host_byte_sex());
	}

	for (i = 0; i < st->nsyms; i++)
	{
		if (ofile->mh != NULL)
		{
			make_symbol_32(&symbol, all_symbols + i);
			symbol.nl_addr = i * sizeof(struct nlist) + st->symoff;
		}
		else {
			make_symbol_64(&symbol, all_symbols64 + i);
			symbol.nl_addr = i * sizeof(struct nlist_64) + st->symoff;
		}
		selected_symbols[(*nsymbols)++] = symbol;
	}

	if (ofile->object_byte_sex != get_host_byte_sex())
	{
		if (ofile->mh != NULL)
			swap_nlist(all_symbols, st->nsyms, ofile->object_byte_sex);
		else
			swap_nlist_64(all_symbols64, st->nsyms, ofile->object_byte_sex);
	}
	/*
	 * Could reallocate selected symbols to the exact size but it is more
	 * of a time waste than a memory savings.
	 */
	return(selected_symbols);
}

static
void
make_symbol_32(
struct symbol *symbol,
struct nlist *nl)
{
	symbol->nl.n_un.n_strx = nl->n_un.n_strx;
	symbol->nl.n_type = nl->n_type;
	symbol->nl.n_sect = nl->n_sect;
	symbol->nl.n_desc = nl->n_desc;
	symbol->nl.n_value = nl->n_value;
}

static
void
make_symbol_64(
struct symbol *symbol,
struct nlist_64 *nl)
{
	symbol->nl = *nl;
}

static char *strings = NULL;
static uint32_t strsize = 0;
/*
 * ofile_processor() is called by ofile_process() for each ofile to process.
 * All ofiles that are object files are process by section non-object files
 * have their logical contents processed entirely.  The locical contents may
 * be a single archive member in an archive, if the name "libx.a(x.o)" was
 * used or a specific architecture if "-arch <arch_flag> fatfile" was used
 * which is not the entire physical file.  If the entire physical file is
 * wanted to be searched then the "-" option is used and this routine is not
 * used.
 */
static void
ofile_processor(
	struct ofile *ofile,
	char *arch_name,
	void *cookie)
{
	char *addr;
	uint32_t offset, size, i, j;
	uint32_t ncmds;
	struct flags *flags;
	struct load_command *lc;
	struct segment_command *sg;
	struct segment_command_64 *sg64;
	struct section *s;
	struct section_64 *s64;
	struct symtab_command *st;
	struct symbol *symbols;
    uint32_t nsymbols;

	flags = (struct flags *)cookie;

	/*
	 * If the ofile is not an object file then process it without reguard
	 * to sections.
	 */
	if (ofile->object_addr == NULL)
	{
		warning("not macho file");
		return;
	}

	/*
	 * The ofile is an object file so process with reguard to it's sections.
	 */
	lc = ofile->load_commands;
	if (ofile->mh != NULL)
		ncmds = ofile->mh->ncmds;
	else
		ncmds = ofile->mh64->ncmds;
	for (i = 0; i < ncmds; i++)
	{
		if (lc->cmd == LC_SYMTAB) {
			st = (struct symtab_command *)lc;
			if (st == NULL || st->nsyms == 0)
			{
				warning("no name list");
				return;
			}

			/* select symbols to print */
			symbols = select_symbols(ofile, st, &nsymbols);

				 	/* set names in the symbols to be printed */
			strings = ofile->object_addr + st->stroff;
			strsize = st->strsize;
			for (i = 0; i < nsymbols; i++)
			{
				if (symbols[i].nl.n_un.n_strx == 0)
					symbols[i].name = "";
				else if ((int)symbols[i].nl.n_un.n_strx < 0 ||
						 (uint32_t)symbols[i].nl.n_un.n_strx > st->strsize)
					symbols[i].name = "bad string index";
				else
					symbols[i].name = symbols[i].nl.n_un.n_strx + strings;

				if ((symbols[i].nl.n_type & N_STAB) == 0 &&
					(symbols[i].nl.n_type & N_TYPE) == N_INDR)
				{
					if (symbols[i].nl.n_value == 0)
						symbols[i].indr_name = "";
					else if (symbols[i].nl.n_value > st->strsize)
						symbols[i].indr_name = "bad string index";
					else
						symbols[i].indr_name = strings + symbols[i].nl.n_value;
				}

				// 测试打印
				if ((symbols[i].nl.n_type & N_TYPE) == N_SECT) {
					printf("%016x %s\n", symbols[i].nl_addr, symbols[i].name);
				}
			}
			free(symbols);
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
}
