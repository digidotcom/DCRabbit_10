/*
 * MIBTEST.C - Excercise MIB database functions.
 * Copyright 2007, Rabbit Semiconductor
 *
 * This demo shows only the MIB (Management Information Base) part of
 * the SNMP suite.  It allows you to interactively add and delte objects,
 * and examine the MIB tree.
 *
 */

#memmap xmem
//#define MIB_DEBUG
//#define MIB_VERBOSE

#define SNMP_ENTERPRISE	12807

#use "mib.lib"

void help()
{
	printf("Cmds:\n");
	printf("  [=]In         Set or add short int\n");
	printf("  [=]Ln         Set or add long int\n");
	printf("  [=]Sstr       Set or add string\n");
	printf("  [=]Ooid       Set or add OID\n");
	printf("  n[.n]...      Append OID level(s)\n");
	printf("  Gn[.n]...     Set absolute OID\n");
	printf("  .             (dot) Go up one OID level\n");
	printf("  ..            (dotdot) Go up two OID levels, etc.\n");
	printf("  N[oid]        Get next after current or given OID\n");
	printf("  ?[oid]        Search for current or given OID\n");
	printf("  D[oid]        Delete current or given subtree\n");
	printf("  X             Toggle xmem string storage\n");
	printf("  <enter>       Print tree\n");
	printf("  Q             Quit\n");
	printf("  H             Print these commands\n");
}

void printstem(snmp_parms * p)
{
	auto word i;

	printf("Stem (length %u):\n", p->stem.len);
	for (i = 0; i < p->stem.len; i++)
		printf(" %02X", (word)p->stem.oid[i]);
	printf("\n%s\n", snmp_format_oid(&p->stem));

}

int main()
{
	auto snmp_parms p, p2;
	auto mib_cursor k;
	auto char c;
	auto int i;
	auto long L;
	auto word rc;
	static int ival[16];
	auto int next_ival;
	static long lval[16];
	auto int next_lval;
	static char sval[16][32];
	auto int next_sval;
	static char cmd[80];
	auto int next_oval;
	static snmp_oid oval[16];
	auto int in_xmem;
	auto snmp_oid o;
	static char sbuf[128];

	snmp_init_parms(&p);

	next_ival = 0;
	next_lval = 0;
	next_sval = 0;
	next_oval = 0;
	in_xmem = 0;
	help();
	for (;;) {
		printf("%s> ", snmp_format_oid(&p.stem));
		gets(cmd);
		c = toupper(cmd[0]);
		if (isdigit(c)) {
			if (!snmp_append_parse_stem(&p, cmd))
				printf("OID parse error\n");
			//printstem(&p);
		}
		else switch (c) {
			case 'G':
				if (!snmp_set_parse_stem(&p, cmd+1))
					printf("OID parse error\n");
				//printstem(&p);
				break;
			case 'X':
				in_xmem = !in_xmem;
				if (in_xmem)
					printf("XMEM string mode on\n");
				else
					printf("XMEM string mode off\n");
				break;
			case '=':
				switch (toupper(cmd[1])) {
					case 'I':
						i = atoi(cmd+2);
						if (!snmp_set_int(&p, i))
							printf("Error setting short int\n");
						break;
					case 'L':
						L = atol(cmd+2);
						if (!snmp_set_long(&p, L))
							printf("Error setting long int\n");
						break;
					case 'S':
						if (!snmp_set_str(&p, cmd+2))
							printf("Error setting string\n");
						break;
					case 'O':
						if (!snmp_set_parse_oid(&o, cmd+2) || !snmp_set_objectID(&p, &o))
							printf("Error setting OID\n");
						break;

				}
				break;
			case 'I':
				ival[next_ival] = atoi(cmd+1);
				if (!snmp_add_int(&p, NULL, ival + next_ival))
					printf("Insert failed\n");
				next_ival = (next_ival + 1) & 0x0F;
				break;
			case 'L':
				lval[next_lval] = atol(cmd+1);
				if (!snmp_add_long(&p, NULL, lval + next_lval))
					printf("Insert failed\n");
				next_lval = (next_lval + 1) & 0x0F;
				break;
			case 'S':
				strncpy(sval[next_sval], cmd+1, 32);
				if (in_xmem) {
					if (!snmp_add_xstr(&p, NULL, paddr(sval[next_sval]), 32))
						printf("Insert failed\n");
				}
				else
					if (!snmp_add_str(&p, NULL, sval[next_sval], 32))
						printf("Insert failed\n");
				next_sval = (next_sval + 1) & 0x0F;
				break;
			case 'O':
				if (!snmp_set_parse_oid(oval + next_oval, cmd+1)) {
					printf("Parse failed\n");
					break;
				}
				if (in_xmem) {
					if (!snmp_add_xobjectID(&p, NULL, paddr(oval + next_oval)))
						printf("Insert failed\n");
				}
				else
					if (!snmp_add_objectID(&p, NULL, oval + next_oval))
						printf("Insert failed\n");
				next_oval = (next_oval + 1) & 0x0F;
				break;
			case '.':
				for (i = 0; i < 32; i++) {
					if (cmd[i] == '.')
						snmp_up_stem(&p, 1);
					else
						break;
				}
				//printstem(&p);
				break;
			case 'D':
				memcpy(&p2, &p, sizeof(p2));
				if (cmd[1] && !snmp_set_parse_stem(&p2, cmd+1)) {
					printf("Parse error\n");
					break;
				}
				if (!snmp_delete(&p2))
					printf("Delete failed\n");
				break;
			case 'N':
				if (cmd[1] && !snmp_set_parse_stem(&p, cmd+1)) {
					printf("Parse error\n");
					break;
				}
				if (!snmp_get_next(&p))
					printf("Get next failed\n");
				else
					printf("Next node number = %u\n", p.index);
				//printstem(&p);
				break;
			case '?':
				if (cmd[1]) {
					if (!snmp_set_parse_oid(&k.s_oid, cmd+1)) {
						printf("Parse error\n");
						break;
					}
				}
				else
					memcpy(&k.s_oid, &p.stem, sizeof(snmp_oid));
				k.index = _mib.Root;
				k.s_offs = 0;
				rc = _mib_find(&k);
				if (!rc)
					printf("No match (insert after %u)", k.index);
				else
					printf("index=%u s_offs=%u n_offs=%u ", k.index, k.s_offs, k.n_offs);
				if (rc & MIB_MATCH)
					printf("Match ");
				if (rc & MIB_N_MATCH)
					printf("Node-compl ");
				if (rc & MIB_S_MATCH)
					printf("Search-compl ");
				if (rc & MIB_S_OFLO)
					printf("Overflow ");
				if (rc & MIB_NOT_LEAF)
					printf("Not-leaf ");
				if (rc & MIB_S_LOW)
					printf("Search-low");
				printf("\n");
				if (rc & MIB_S_MATCH && !(rc & (MIB_S_OFLO | MIB_NOT_LEAF))) {
					memcpy(&p2, &p, sizeof(p2));
					if (cmd[1] && !snmp_set_parse_stem(&p2, cmd+1)) {
						printf("Parse error\n");
						break;
					}
					if (!snmp_get(&p2)) {
						printf("Retrieval error\n");
						break;
					}
					printf("Leaf node: type=%d\n", snmp_last_type(&p2));
					if (snmp_last_type(&p2) == SNMP_SHORT)
						printf("  short int %d\n", snmp_last_int(&p2));
					else if (snmp_last_type(&p2) == SNMP_LONG)
						printf("  long int %ld\n", snmp_last_long(&p2));
					else if (snmp_last_type(&p2) == SNMP_STR) {
						xmem2root(sbuf, snmp_last_xmem(&p2), sizeof(sbuf));
						printf("  str \"%s\"\n", sbuf);
					}
					else if (snmp_last_type(&p2) == SNMP_OID) {
						snmp_last_objectID(&p2, &o);
						printf("  oid %s\n", snmp_format_oid(&o));
					}
				}
				break;
			case 'Q':
				exit(0);
			case 'H':
				help();
				break;
			default:
				snmp_print_tree();
		}

	}

	return 0;
}