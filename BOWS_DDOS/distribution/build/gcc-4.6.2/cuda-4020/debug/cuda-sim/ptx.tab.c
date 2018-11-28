/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         ptx_parse
#define yylex           ptx_lex
#define yyerror         ptx_error
#define yylval          ptx_lval
#define yychar          ptx_char
#define yydebug         ptx_debug
#define yynerrs         ptx_nerrs


/* Copy the first part of user declarations.  */


/* Line 268 of yacc.c  */
#line 79 "/home/ashafiey/workspace/depot/gpgpu_sim_research/mimdlike/distribution/build/gcc-4.6.2/cuda-4020/debug/cuda-sim/ptx.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRING = 258,
     OPCODE = 259,
     ALIGN_DIRECTIVE = 260,
     BRANCHTARGETS_DIRECTIVE = 261,
     BYTE_DIRECTIVE = 262,
     CALLPROTOTYPE_DIRECTIVE = 263,
     CALLTARGETS_DIRECTIVE = 264,
     CONST_DIRECTIVE = 265,
     CONSTPTR_DIRECTIVE = 266,
     PTR_DIRECTIVE = 267,
     ENTRY_DIRECTIVE = 268,
     EXTERN_DIRECTIVE = 269,
     FILE_DIRECTIVE = 270,
     FUNC_DIRECTIVE = 271,
     GLOBAL_DIRECTIVE = 272,
     LOCAL_DIRECTIVE = 273,
     LOC_DIRECTIVE = 274,
     MAXNCTAPERSM_DIRECTIVE = 275,
     MAXNNREG_DIRECTIVE = 276,
     MAXNTID_DIRECTIVE = 277,
     MINNCTAPERSM_DIRECTIVE = 278,
     PARAM_DIRECTIVE = 279,
     PRAGMA_DIRECTIVE = 280,
     REG_DIRECTIVE = 281,
     REQNTID_DIRECTIVE = 282,
     SECTION_DIRECTIVE = 283,
     SHARED_DIRECTIVE = 284,
     SREG_DIRECTIVE = 285,
     STRUCT_DIRECTIVE = 286,
     SURF_DIRECTIVE = 287,
     TARGET_DIRECTIVE = 288,
     TEX_DIRECTIVE = 289,
     UNION_DIRECTIVE = 290,
     VERSION_DIRECTIVE = 291,
     ADDRESS_SIZE_DIRECTIVE = 292,
     VISIBLE_DIRECTIVE = 293,
     IDENTIFIER = 294,
     INT_OPERAND = 295,
     FLOAT_OPERAND = 296,
     DOUBLE_OPERAND = 297,
     S8_TYPE = 298,
     S16_TYPE = 299,
     S32_TYPE = 300,
     S64_TYPE = 301,
     U8_TYPE = 302,
     U16_TYPE = 303,
     U32_TYPE = 304,
     U64_TYPE = 305,
     F16_TYPE = 306,
     F32_TYPE = 307,
     F64_TYPE = 308,
     FF64_TYPE = 309,
     B8_TYPE = 310,
     B16_TYPE = 311,
     B32_TYPE = 312,
     B64_TYPE = 313,
     BB64_TYPE = 314,
     BB128_TYPE = 315,
     PRED_TYPE = 316,
     TEXREF_TYPE = 317,
     SAMPLERREF_TYPE = 318,
     SURFREF_TYPE = 319,
     V2_TYPE = 320,
     V3_TYPE = 321,
     V4_TYPE = 322,
     COMMA = 323,
     PRED = 324,
     HALF_OPTION = 325,
     EQ_OPTION = 326,
     NE_OPTION = 327,
     LT_OPTION = 328,
     LE_OPTION = 329,
     GT_OPTION = 330,
     GE_OPTION = 331,
     LO_OPTION = 332,
     LS_OPTION = 333,
     HI_OPTION = 334,
     HS_OPTION = 335,
     EQU_OPTION = 336,
     NEU_OPTION = 337,
     LTU_OPTION = 338,
     LEU_OPTION = 339,
     GTU_OPTION = 340,
     GEU_OPTION = 341,
     NUM_OPTION = 342,
     NAN_OPTION = 343,
     CF_OPTION = 344,
     SF_OPTION = 345,
     NSF_OPTION = 346,
     LEFT_SQUARE_BRACKET = 347,
     RIGHT_SQUARE_BRACKET = 348,
     WIDE_OPTION = 349,
     SPECIAL_REGISTER = 350,
     MINUS = 351,
     PLUS = 352,
     COLON = 353,
     SEMI_COLON = 354,
     EXCLAMATION = 355,
     PIPE = 356,
     RIGHT_BRACE = 357,
     LEFT_BRACE = 358,
     EQUALS = 359,
     PERIOD = 360,
     BACKSLASH = 361,
     DIMENSION_MODIFIER = 362,
     RN_OPTION = 363,
     RZ_OPTION = 364,
     RM_OPTION = 365,
     RP_OPTION = 366,
     RNI_OPTION = 367,
     RZI_OPTION = 368,
     RMI_OPTION = 369,
     RPI_OPTION = 370,
     UNI_OPTION = 371,
     GEOM_MODIFIER_1D = 372,
     GEOM_MODIFIER_2D = 373,
     GEOM_MODIFIER_3D = 374,
     SAT_OPTION = 375,
     FTZ_OPTION = 376,
     NEG_OPTION = 377,
     SYNC_OPTION = 378,
     RED_OPTION = 379,
     POPC_REDUCTION = 380,
     AND_REDUCTION = 381,
     OR_REDUCTION = 382,
     ARRIVE_OPTION = 383,
     ATOMIC_AND = 384,
     ATOMIC_OR = 385,
     ATOMIC_XOR = 386,
     ATOMIC_CAS = 387,
     ATOMIC_EXCH = 388,
     ATOMIC_CAS_SYNC = 389,
     ATOMIC_EXCH_SYNC = 390,
     ATOMIC_ADD = 391,
     ATOMIC_INC = 392,
     ATOMIC_DEC = 393,
     ATOMIC_MIN = 394,
     ATOMIC_MAX = 395,
     LEFT_ANGLE_BRACKET = 396,
     RIGHT_ANGLE_BRACKET = 397,
     LEFT_PAREN = 398,
     RIGHT_PAREN = 399,
     APPROX_OPTION = 400,
     FULL_OPTION = 401,
     ANY_OPTION = 402,
     ALL_OPTION = 403,
     BALLOT_OPTION = 404,
     GLOBAL_OPTION = 405,
     CTA_OPTION = 406,
     SYS_OPTION = 407,
     EXIT_OPTION = 408,
     ABS_OPTION = 409,
     TO_OPTION = 410,
     CA_OPTION = 411,
     CG_OPTION = 412,
     CS_OPTION = 413,
     LU_OPTION = 414,
     CV_OPTION = 415,
     WB_OPTION = 416,
     WT_OPTION = 417
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 30 "ptx.y"

  double double_value;
  float  float_value;
  int    int_value;
  char * string_value;
  void * ptr_value;



/* Line 293 of yacc.c  */
#line 287 "/home/ashafiey/workspace/depot/gpgpu_sim_research/mimdlike/distribution/build/gcc-4.6.2/cuda-4020/debug/cuda-sim/ptx.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */

/* Line 343 of yacc.c  */
#line 202 "ptx.y"

  	#include "ptx_parser.h"
	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
	void syntax_not_implemented();
	extern int g_func_decl;
	int ptx_lex(void);
	int ptx_error(const char *);


/* Line 343 of yacc.c  */
#line 311 "/home/ashafiey/workspace/depot/gpgpu_sim_research/mimdlike/distribution/build/gcc-4.6.2/cuda-4020/debug/cuda-sim/ptx.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   592

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  163
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  64
/* YYNRULES -- Number of rules.  */
#define YYNRULES  277
/* YYNRULES -- Number of states.  */
#define YYNSTATES  383

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   417

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    13,    14,    18,    19,
      20,    26,    33,    36,    39,    41,    44,    45,    46,    54,
      55,    59,    61,    62,    63,    70,    72,    74,    77,    79,
      82,    85,    86,    88,    89,    94,    95,   101,   102,   107,
     108,   112,   115,   117,   119,   121,   124,   128,   130,   132,
     135,   138,   141,   143,   146,   149,   153,   156,   161,   168,
     171,   175,   180,   184,   187,   190,   195,   200,   207,   209,
     211,   215,   217,   222,   226,   231,   233,   236,   238,   240,
     242,   244,   247,   249,   251,   253,   255,   257,   259,   261,
     263,   265,   267,   269,   272,   274,   276,   278,   280,   282,
     284,   286,   288,   290,   292,   294,   296,   298,   300,   302,
     304,   306,   308,   310,   312,   314,   316,   318,   320,   322,
     326,   330,   332,   336,   339,   342,   346,   347,   359,   366,
     372,   375,   377,   378,   382,   384,   387,   391,   395,   399,
     403,   407,   411,   415,   419,   423,   427,   431,   435,   437,
     440,   442,   444,   446,   448,   450,   452,   454,   456,   458,
     460,   462,   464,   466,   468,   470,   472,   474,   476,   478,
     480,   482,   484,   486,   488,   490,   492,   494,   496,   498,
     500,   502,   504,   506,   508,   510,   512,   514,   516,   518,
     520,   522,   524,   526,   528,   530,   532,   534,   536,   538,
     540,   542,   544,   546,   548,   550,   552,   554,   556,   558,
     560,   562,   564,   566,   568,   570,   572,   574,   576,   578,
     580,   582,   584,   586,   588,   590,   592,   594,   596,   598,
     602,   604,   607,   610,   612,   614,   616,   618,   621,   623,
     627,   630,   634,   637,   641,   645,   650,   655,   659,   664,
     669,   675,   683,   693,   697,   698,   705,   708,   710,   714,
     719,   724,   729,   732,   736,   741,   746,   751,   757,   763,
     768,   770,   772,   774,   776,   779,   782,   786
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     164,     0,    -1,    -1,   164,   189,    -1,   164,   165,    -1,
     164,   171,    -1,    -1,   171,   166,   187,    -1,    -1,    -1,
     171,   167,   170,   168,   187,    -1,    22,    40,    68,    40,
      68,    40,    -1,    23,    40,    -1,    20,    40,    -1,   169,
      -1,   170,   169,    -1,    -1,    -1,   178,   143,   172,   181,
     144,   173,   175,    -1,    -1,   178,   174,   175,    -1,   178,
      -1,    -1,    -1,    39,   176,   143,   177,   179,   144,    -1,
      39,    -1,    13,    -1,    38,    13,    -1,    16,    -1,    38,
      16,    -1,    14,    16,    -1,    -1,   181,    -1,    -1,   179,
      68,   180,   181,    -1,    -1,    24,   182,   191,   184,   193,
      -1,    -1,    26,   183,   191,   193,    -1,    -1,    12,   185,
     186,    -1,    12,   186,    -1,    17,    -1,    18,    -1,    29,
      -1,     5,    40,    -1,   103,   188,   102,    -1,   189,    -1,
     204,    -1,   188,   189,    -1,   188,   204,    -1,   188,   187,
      -1,   187,    -1,   190,    99,    -1,    36,    42,    -1,    36,
      42,    97,    -1,    37,    40,    -1,    33,    39,    68,    39,
      -1,    33,    39,    68,    39,    68,    39,    -1,    33,    39,
      -1,    15,    40,     3,    -1,    19,    40,    40,    40,    -1,
      25,     3,    99,    -1,   171,    99,    -1,   191,   192,    -1,
     191,   193,   104,   202,    -1,   191,   193,   104,   225,    -1,
      11,    39,    68,    39,    68,    40,    -1,   194,    -1,   193,
      -1,   192,    68,   193,    -1,    39,    -1,    39,   141,    40,
     142,    -1,    39,    92,    93,    -1,    39,    92,    40,    93,
      -1,   195,    -1,   194,   195,    -1,   197,    -1,   199,    -1,
     196,    -1,    14,    -1,     5,    40,    -1,    26,    -1,    30,
      -1,   198,    -1,    10,    -1,    17,    -1,    18,    -1,    24,
      -1,    29,    -1,    32,    -1,    34,    -1,   201,    -1,   200,
     201,    -1,    65,    -1,    66,    -1,    67,    -1,    43,    -1,
      44,    -1,    45,    -1,    46,    -1,    47,    -1,    48,    -1,
      49,    -1,    50,    -1,    51,    -1,    52,    -1,    53,    -1,
      54,    -1,    55,    -1,    56,    -1,    57,    -1,    58,    -1,
      59,    -1,    60,    -1,    61,    -1,    62,    -1,    63,    -1,
      64,    -1,   103,   203,   102,    -1,   103,   202,   102,    -1,
     225,    -1,   203,    68,   225,    -1,   205,    99,    -1,    39,
      98,    -1,   209,   205,    99,    -1,    -1,   207,   143,   218,
     144,   206,    68,   218,    68,   143,   217,   144,    -1,   207,
     218,    68,   143,   217,   144,    -1,   207,   218,    68,   143,
     144,    -1,   207,   217,    -1,   207,    -1,    -1,     4,   208,
     210,    -1,     4,    -1,    69,    39,    -1,    69,   100,    39,
      -1,    69,    39,    73,    -1,    69,    39,    71,    -1,    69,
      39,    74,    -1,    69,    39,    72,    -1,    69,    39,    76,
      -1,    69,    39,    81,    -1,    69,    39,    85,    -1,    69,
      39,    82,    -1,    69,    39,    89,    -1,    69,    39,    90,
      -1,    69,    39,    91,    -1,   211,    -1,   211,   210,    -1,
     199,    -1,   216,    -1,   198,    -1,   213,    -1,   123,    -1,
     128,    -1,   124,    -1,   125,    -1,   126,    -1,   127,    -1,
     116,    -1,    94,    -1,   147,    -1,   148,    -1,   149,    -1,
     150,    -1,   151,    -1,   152,    -1,   117,    -1,   118,    -1,
     119,    -1,   120,    -1,   121,    -1,   122,    -1,   145,    -1,
     146,    -1,   153,    -1,   154,    -1,   212,    -1,   155,    -1,
      70,    -1,   156,    -1,   157,    -1,   158,    -1,   159,    -1,
     160,    -1,   161,    -1,   162,    -1,   129,    -1,   130,    -1,
     131,    -1,   132,    -1,   133,    -1,   134,    -1,   135,    -1,
     136,    -1,   137,    -1,   138,    -1,   139,    -1,   140,    -1,
     214,    -1,   215,    -1,   108,    -1,   109,    -1,   110,    -1,
     111,    -1,   112,    -1,   113,    -1,   114,    -1,   115,    -1,
      71,    -1,    72,    -1,    73,    -1,    74,    -1,    75,    -1,
      76,    -1,    77,    -1,    78,    -1,    79,    -1,    80,    -1,
      81,    -1,    82,    -1,    83,    -1,    84,    -1,    85,    -1,
      86,    -1,    87,    -1,    88,    -1,   218,    -1,   218,    68,
     217,    -1,    39,    -1,   100,    39,    -1,    96,    39,    -1,
     223,    -1,   225,    -1,   222,    -1,   219,    -1,    96,   219,
      -1,   220,    -1,    39,    97,    40,    -1,    39,    77,    -1,
      96,    39,    77,    -1,    39,    79,    -1,    96,    39,    79,
      -1,    39,   101,    39,    -1,    39,   101,    39,    77,    -1,
      39,   101,    39,    79,    -1,    39,   106,    39,    -1,    39,
     106,    39,    77,    -1,    39,   106,    39,    79,    -1,   103,
      39,    68,    39,   102,    -1,   103,    39,    68,    39,    68,
      39,   102,    -1,   103,    39,    68,    39,    68,    39,    68,
      39,   102,    -1,   103,    39,   102,    -1,    -1,    92,    39,
      68,   221,   219,    93,    -1,    95,   107,    -1,    95,    -1,
      92,   226,    93,    -1,    39,    92,   226,    93,    -1,    39,
      92,   225,    93,    -1,    39,    92,   224,    93,    -1,    96,
     223,    -1,    39,    97,    39,    -1,    39,    97,    39,    77,
      -1,    39,    97,    39,    79,    -1,    39,    97,   104,    39,
      -1,    39,    97,   104,    39,    77,    -1,    39,    97,   104,
      39,    79,    -1,    39,    97,   104,    40,    -1,    40,    -1,
      41,    -1,    42,    -1,    39,    -1,    39,    77,    -1,    39,
      79,    -1,    39,    97,    40,    -1,    40,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   215,   215,   216,   217,   218,   221,   221,   222,   222,
     222,   225,   228,   229,   232,   233,   236,   236,   236,   237,
     237,   238,   241,   241,   241,   242,   245,   246,   247,   248,
     249,   252,   253,   254,   254,   256,   256,   257,   257,   259,
     260,   261,   263,   264,   265,   267,   269,   271,   272,   273,
     274,   275,   276,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   292,   293,   294,   295,   298,   300,
     301,   303,   304,   316,   317,   320,   321,   323,   324,   325,
     326,   329,   331,   332,   333,   336,   337,   338,   339,   340,
     341,   342,   345,   346,   349,   350,   351,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   378,
     379,   381,   382,   384,   385,   386,   388,   388,   389,   390,
     391,   392,   395,   395,   396,   398,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,   413,   414,
     416,   417,   418,   419,   420,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     470,   471,   473,   474,   475,   476,   479,   480,   481,   482,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   505,   506,
     508,   509,   510,   511,   512,   513,   514,   515,   516,   517,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     530,   531,   532,   533,   536,   536,   541,   542,   545,   546,
     547,   548,   549,   552,   553,   554,   555,   556,   557,   558,
     561,   562,   563,   566,   567,   568,   569,   570
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "STRING", "OPCODE", "ALIGN_DIRECTIVE",
  "BRANCHTARGETS_DIRECTIVE", "BYTE_DIRECTIVE", "CALLPROTOTYPE_DIRECTIVE",
  "CALLTARGETS_DIRECTIVE", "CONST_DIRECTIVE", "CONSTPTR_DIRECTIVE",
  "PTR_DIRECTIVE", "ENTRY_DIRECTIVE", "EXTERN_DIRECTIVE", "FILE_DIRECTIVE",
  "FUNC_DIRECTIVE", "GLOBAL_DIRECTIVE", "LOCAL_DIRECTIVE", "LOC_DIRECTIVE",
  "MAXNCTAPERSM_DIRECTIVE", "MAXNNREG_DIRECTIVE", "MAXNTID_DIRECTIVE",
  "MINNCTAPERSM_DIRECTIVE", "PARAM_DIRECTIVE", "PRAGMA_DIRECTIVE",
  "REG_DIRECTIVE", "REQNTID_DIRECTIVE", "SECTION_DIRECTIVE",
  "SHARED_DIRECTIVE", "SREG_DIRECTIVE", "STRUCT_DIRECTIVE",
  "SURF_DIRECTIVE", "TARGET_DIRECTIVE", "TEX_DIRECTIVE", "UNION_DIRECTIVE",
  "VERSION_DIRECTIVE", "ADDRESS_SIZE_DIRECTIVE", "VISIBLE_DIRECTIVE",
  "IDENTIFIER", "INT_OPERAND", "FLOAT_OPERAND", "DOUBLE_OPERAND",
  "S8_TYPE", "S16_TYPE", "S32_TYPE", "S64_TYPE", "U8_TYPE", "U16_TYPE",
  "U32_TYPE", "U64_TYPE", "F16_TYPE", "F32_TYPE", "F64_TYPE", "FF64_TYPE",
  "B8_TYPE", "B16_TYPE", "B32_TYPE", "B64_TYPE", "BB64_TYPE", "BB128_TYPE",
  "PRED_TYPE", "TEXREF_TYPE", "SAMPLERREF_TYPE", "SURFREF_TYPE", "V2_TYPE",
  "V3_TYPE", "V4_TYPE", "COMMA", "PRED", "HALF_OPTION", "EQ_OPTION",
  "NE_OPTION", "LT_OPTION", "LE_OPTION", "GT_OPTION", "GE_OPTION",
  "LO_OPTION", "LS_OPTION", "HI_OPTION", "HS_OPTION", "EQU_OPTION",
  "NEU_OPTION", "LTU_OPTION", "LEU_OPTION", "GTU_OPTION", "GEU_OPTION",
  "NUM_OPTION", "NAN_OPTION", "CF_OPTION", "SF_OPTION", "NSF_OPTION",
  "LEFT_SQUARE_BRACKET", "RIGHT_SQUARE_BRACKET", "WIDE_OPTION",
  "SPECIAL_REGISTER", "MINUS", "PLUS", "COLON", "SEMI_COLON",
  "EXCLAMATION", "PIPE", "RIGHT_BRACE", "LEFT_BRACE", "EQUALS", "PERIOD",
  "BACKSLASH", "DIMENSION_MODIFIER", "RN_OPTION", "RZ_OPTION", "RM_OPTION",
  "RP_OPTION", "RNI_OPTION", "RZI_OPTION", "RMI_OPTION", "RPI_OPTION",
  "UNI_OPTION", "GEOM_MODIFIER_1D", "GEOM_MODIFIER_2D", "GEOM_MODIFIER_3D",
  "SAT_OPTION", "FTZ_OPTION", "NEG_OPTION", "SYNC_OPTION", "RED_OPTION",
  "POPC_REDUCTION", "AND_REDUCTION", "OR_REDUCTION", "ARRIVE_OPTION",
  "ATOMIC_AND", "ATOMIC_OR", "ATOMIC_XOR", "ATOMIC_CAS", "ATOMIC_EXCH",
  "ATOMIC_CAS_SYNC", "ATOMIC_EXCH_SYNC", "ATOMIC_ADD", "ATOMIC_INC",
  "ATOMIC_DEC", "ATOMIC_MIN", "ATOMIC_MAX", "LEFT_ANGLE_BRACKET",
  "RIGHT_ANGLE_BRACKET", "LEFT_PAREN", "RIGHT_PAREN", "APPROX_OPTION",
  "FULL_OPTION", "ANY_OPTION", "ALL_OPTION", "BALLOT_OPTION",
  "GLOBAL_OPTION", "CTA_OPTION", "SYS_OPTION", "EXIT_OPTION", "ABS_OPTION",
  "TO_OPTION", "CA_OPTION", "CG_OPTION", "CS_OPTION", "LU_OPTION",
  "CV_OPTION", "WB_OPTION", "WT_OPTION", "$accept", "input",
  "function_defn", "$@1", "$@2", "$@3", "block_spec", "block_spec_list",
  "function_decl", "$@4", "$@5", "$@6", "function_ident_param", "$@7",
  "$@8", "function_decl_header", "param_list", "$@9", "param_entry",
  "$@10", "$@11", "ptr_spec", "ptr_space_spec", "ptr_align_spec",
  "statement_block", "statement_list", "directive_statement",
  "variable_declaration", "variable_spec", "identifier_list",
  "identifier_spec", "var_spec_list", "var_spec", "align_spec",
  "space_spec", "addressable_spec", "type_spec", "vector_spec",
  "scalar_type", "initializer_list", "literal_list",
  "instruction_statement", "instruction", "$@12", "opcode_spec", "$@13",
  "pred_spec", "option_list", "option", "atomic_operation_spec",
  "rounding_mode", "floating_point_rounding_mode", "integer_rounding_mode",
  "compare_spec", "operand_list", "operand", "vector_operand",
  "tex_operand", "$@14", "builtin_operand", "memory_operand",
  "twin_operand", "literal_operand", "address_expression", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   163,   164,   164,   164,   164,   166,   165,   167,   168,
     165,   169,   169,   169,   170,   170,   172,   173,   171,   174,
     171,   171,   176,   177,   175,   175,   178,   178,   178,   178,
     178,   179,   179,   180,   179,   182,   181,   183,   181,   184,
     184,   184,   185,   185,   185,   186,   187,   188,   188,   188,
     188,   188,   188,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   190,   190,   190,   190,   191,   192,
     192,   193,   193,   193,   193,   194,   194,   195,   195,   195,
     195,   196,   197,   197,   197,   198,   198,   198,   198,   198,
     198,   198,   199,   199,   200,   200,   200,   201,   201,   201,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   202,
     202,   203,   203,   204,   204,   204,   206,   205,   205,   205,
     205,   205,   208,   207,   207,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   210,   210,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     213,   213,   214,   214,   214,   214,   215,   215,   215,   215,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   217,   217,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     219,   219,   219,   219,   221,   220,   222,   222,   223,   223,
     223,   223,   223,   224,   224,   224,   224,   224,   224,   224,
     225,   225,   225,   226,   226,   226,   226,   226
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     0,     3,     0,     0,
       5,     6,     2,     2,     1,     2,     0,     0,     7,     0,
       3,     1,     0,     0,     6,     1,     1,     2,     1,     2,
       2,     0,     1,     0,     4,     0,     5,     0,     4,     0,
       3,     2,     1,     1,     1,     2,     3,     1,     1,     2,
       2,     2,     1,     2,     2,     3,     2,     4,     6,     2,
       3,     4,     3,     2,     2,     4,     4,     6,     1,     1,
       3,     1,     4,     3,     4,     1,     2,     1,     1,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     1,     3,     2,     2,     3,     0,    11,     6,     5,
       2,     1,     0,     3,     1,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     2,     2,     1,     1,     1,     1,     2,     1,     3,
       2,     3,     2,     3,     3,     4,     4,     3,     4,     4,
       5,     7,     9,     3,     0,     6,     2,     1,     3,     4,
       4,     4,     2,     3,     4,     4,     4,     5,     5,     4,
       1,     1,     1,     1,     2,     2,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     1,     0,    85,     0,    26,    80,     0,    28,
      86,    87,     0,    88,     0,    82,    89,    83,    90,     0,
      91,     0,     0,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,    94,    95,    96,     4,
       5,    21,     3,     0,     0,    68,    75,    79,    77,    84,
      78,     0,    92,    81,     0,    30,     0,     0,     0,    59,
      54,    56,    27,    29,    63,     0,     0,    16,     0,    53,
      71,    64,    69,    80,    76,    93,     0,    60,     0,    62,
       0,    55,     0,     7,     0,     0,     0,    14,     9,     0,
      25,    20,     0,     0,     0,     0,     0,    61,    57,   132,
       0,     0,     0,    52,     0,    47,    48,     0,   131,     0,
      13,     0,    12,     0,    15,    35,    37,     0,     0,     0,
      73,     0,    70,   270,   271,   272,     0,    65,    66,     0,
       0,     0,   124,   135,     0,    46,    51,    49,    50,   123,
     230,     0,   257,     0,     0,     0,     0,   130,   228,   236,
     238,   235,   233,   234,     0,     0,    10,     0,     0,    17,
      23,    74,    72,     0,     0,   121,    67,    58,   180,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   161,   202,   203,
     204,   205,   206,   207,   208,   209,   160,   168,   169,   170,
     171,   172,   173,   154,   156,   157,   158,   159,   155,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   174,   175,   162,   163,   164,   165,   166,   167,   176,
     177,   179,   181,   182,   183,   184,   185,   186,   187,   152,
     150,   133,   148,   178,   153,   200,   201,   151,   138,   140,
     137,   139,   141,   142,   144,   143,   145,   146,   147,   136,
     240,   242,     0,     0,     0,     0,   273,   277,     0,   256,
     232,     0,     0,   237,   262,   231,     0,     0,     0,   125,
       0,    39,     0,     0,    31,   120,     0,   119,   149,   273,
     270,     0,     0,     0,   239,   244,   247,   254,   274,   275,
       0,   258,   241,   243,   273,     0,     0,   253,   126,     0,
     229,   228,     0,     0,     0,    38,    18,     0,    32,   122,
       0,   261,   260,   259,   245,   246,   248,   249,     0,   276,
       0,     0,   129,     0,     0,    11,     0,    42,    43,    44,
       0,    41,    36,    33,    24,   263,     0,     0,     0,   250,
       0,   128,    45,    40,     0,   264,   265,   266,   269,   255,
       0,     0,    34,   267,   268,     0,   251,     0,     0,     0,
     252,     0,   127
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    49,    75,    76,   123,    97,    98,   112,    99,
     293,    78,   101,   128,   294,    51,   327,   364,   127,   167,
     168,   324,   350,   351,    93,   114,    52,    53,    54,    81,
      82,    55,    56,    57,    58,    59,    60,    61,    62,   137,
     174,   116,   117,   341,   118,   141,   119,   251,   252,   253,
     254,   255,   256,   257,   320,   321,   159,   160,   338,   161,
     162,   301,   163,   278
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -279
static const yytype_int16 yypact[] =
{
    -279,   419,  -279,   -23,  -279,    53,  -279,    20,    64,  -279,
    -279,  -279,   128,  -279,   161,  -279,  -279,  -279,  -279,   173,
    -279,   185,   175,     6,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
     -10,    26,  -279,   131,   199,   482,  -279,  -279,  -279,  -279,
    -279,   507,  -279,  -279,   187,  -279,   255,   227,   160,   216,
     171,  -279,  -279,  -279,  -279,   196,    80,  -279,   264,  -279,
      73,   236,   203,  -279,  -279,  -279,   272,  -279,   277,  -279,
     279,  -279,   351,  -279,   304,   306,   307,  -279,    80,   -18,
     205,  -279,   100,   309,   199,    -8,   248,  -279,   282,   133,
     253,   -34,   254,  -279,   276,  -279,  -279,   258,   139,   348,
    -279,   286,  -279,   196,  -279,  -279,  -279,   214,   217,   266,
    -279,   221,  -279,  -279,  -279,  -279,    -8,  -279,  -279,   331,
     333,    -3,  -279,   501,   334,  -279,  -279,  -279,  -279,  -279,
     177,   143,   267,    -2,   343,   347,   157,  -279,   323,  -279,
    -279,  -279,  -279,  -279,   293,   353,  -279,   482,   482,  -279,
    -279,  -279,  -279,   319,   102,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,    -3,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,    57,   382,   384,   386,   123,  -279,   335,  -279,
     193,   169,   127,  -279,  -279,  -279,   108,   283,   145,  -279,
     358,   427,   199,   264,   -18,  -279,   222,  -279,  -279,   -59,
    -279,   338,   349,   354,  -279,   -52,   126,  -279,  -279,  -279,
     400,  -279,  -279,  -279,   200,   366,   402,  -279,  -279,   121,
    -279,   378,   410,   189,   199,  -279,  -279,   -57,  -279,  -279,
     -16,  -279,  -279,  -279,  -279,  -279,  -279,  -279,   356,  -279,
     109,   392,  -279,   317,   157,  -279,   448,  -279,  -279,  -279,
     484,  -279,  -279,  -279,  -279,   194,   207,   397,   452,  -279,
     157,  -279,  -279,  -279,   -18,  -279,  -279,   219,  -279,  -279,
     124,   425,  -279,  -279,  -279,   455,  -279,   352,   395,   157,
    -279,   357,  -279
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -279,  -279,  -279,  -279,  -279,  -279,   404,  -279,   497,  -279,
    -279,  -279,   210,  -279,  -279,  -279,  -279,  -279,  -278,  -279,
    -279,  -279,  -279,   154,   152,  -279,    75,  -279,    83,  -279,
    -102,  -279,   450,  -279,  -279,  -113,  -111,  -279,   446,   373,
    -279,   396,   394,  -279,  -279,  -279,  -279,   263,  -279,  -279,
    -279,  -279,  -279,  -279,  -118,  -117,  -150,  -279,  -279,  -279,
    -144,  -279,  -101,   245
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -135
static const yytype_int16 yytable[] =
{
     157,   158,   132,   283,   138,   143,   125,     4,   126,   284,
      -8,   353,    -8,    -8,    10,    11,   328,    63,   308,    72,
     309,    13,    73,   355,   339,   334,    16,   335,   249,    18,
     250,    20,   133,   134,   135,   175,    65,   280,   330,   287,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,   -19,   144,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   372,   354,   356,    74,
     281,   197,    64,    -6,   282,   136,   299,   300,   134,   135,
      94,   155,    95,    96,    66,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   284,   249,
     129,   250,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     150,   133,   134,   135,    68,   102,   315,   115,    67,    77,
     296,   302,  -134,  -134,  -134,  -134,   316,   358,   150,   133,
     134,   135,   276,   277,   150,   133,   134,   135,   357,   147,
     325,   307,   375,   130,   346,   329,   150,   133,   134,   135,
     308,   343,   309,   336,   297,   337,   347,   348,   314,   277,
     317,   359,    69,   151,   103,    71,   152,   153,   349,   281,
     310,   154,   352,   282,   155,  -134,   376,    70,  -134,  -134,
      79,   151,  -134,  -134,   152,   153,  -134,   151,    80,   154,
     152,   153,   155,   371,   113,   154,   367,   368,   155,   151,
     291,   292,   152,   153,   270,    86,   271,   154,    87,    89,
     155,   381,   133,   134,   135,   342,   146,    88,    91,   272,
     312,   365,   313,   366,   273,   166,  -134,   308,   274,   309,
     109,     3,   156,   275,    90,   272,     4,     5,   319,     6,
       7,     8,     9,    10,    11,    12,   373,   310,   374,    92,
      13,    14,    15,   100,   104,    16,    17,   105,    18,    19,
      20,   106,    21,    22,    23,   110,   139,   107,   108,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,   120,   111,   121,   122,   -22,   131,
     140,   142,   109,    74,   165,   109,     3,   149,   169,   171,
     170,     4,     5,   172,     6,     7,     8,     9,    10,    11,
      12,   176,   177,   269,   279,    13,    14,    15,   145,    92,
      16,    17,   285,    18,    19,    20,   286,    21,    22,    23,
     110,   288,   289,   290,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,     2,
     111,   295,   304,   305,     3,   306,   322,   318,   311,     4,
       5,   331,     6,     7,     8,     9,    10,    11,    12,   323,
     339,   340,   332,    13,    14,    15,   344,   333,    16,    17,
     345,    18,    19,    20,    92,    21,    22,    23,   272,   155,
     360,   361,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,     3,   362,   346,
     369,   370,     4,   377,   378,   379,    83,   380,    50,    10,
      11,   382,   124,   326,   363,    84,    13,    85,    15,   173,
     148,    16,    17,   164,    18,   298,    20,   303,     0,     0,
       0,     0,     0,     0,     0,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,   258,   259,   260,   261,     0,   262,     0,     0,
       0,     0,   263,   264,     0,     0,   265,     0,     0,     0,
     266,   267,   268
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-279))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
     118,   118,   104,   153,   105,    39,    24,    10,    26,   153,
      20,    68,    22,    23,    17,    18,   294,    40,    77,    13,
      79,    24,    16,    39,    40,    77,    29,    79,   141,    32,
     141,    34,    40,    41,    42,   136,    16,    39,    97,   156,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    39,   100,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,   364,   144,   104,    99,
      92,    94,    39,   103,    96,   103,    39,    40,    41,    42,
      20,   103,    22,    23,    40,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   282,   252,
      40,   252,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
      39,    40,    41,    42,     3,    92,    39,    92,    40,   143,
      68,   272,    39,    40,    41,    42,    68,    68,    39,    40,
      41,    42,    39,    40,    39,    40,    41,    42,   338,   114,
     292,    68,    68,    93,     5,   296,    39,    40,    41,    42,
      77,   319,    79,    77,   102,    79,    17,    18,    39,    40,
     102,   102,    39,    92,   141,    40,    95,    96,    29,    92,
      97,   100,   324,    96,   103,    92,   102,    42,    95,    96,
      99,    92,    99,   100,    95,    96,   103,    92,    39,   100,
      95,    96,   103,   360,    92,   100,    39,    40,   103,    92,
     167,   168,    95,    96,    77,    68,    79,   100,     3,    99,
     103,   379,    40,    41,    42,   144,   114,    40,    97,    92,
      77,    77,    79,    79,    97,   123,   143,    77,   101,    79,
       4,     5,   143,   106,    68,    92,    10,    11,   143,    13,
      14,    15,    16,    17,    18,    19,    77,    97,    79,   103,
      24,    25,    26,    39,    68,    29,    30,   104,    32,    33,
      34,    39,    36,    37,    38,    39,    68,    40,    39,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    40,    69,    40,    40,   143,    40,
      68,    98,     4,    99,    68,     4,     5,    99,   144,    93,
     143,    10,    11,   142,    13,    14,    15,    16,    17,    18,
      19,    40,    39,    39,   107,    24,    25,    26,   102,   103,
      29,    30,    39,    32,    33,    34,    39,    36,    37,    38,
      39,    68,    99,    40,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,     0,
      69,   102,    40,    39,     5,    39,    68,   144,    93,    10,
      11,    93,    13,    14,    15,    16,    17,    18,    19,    12,
      40,    39,    93,    24,    25,    26,    68,    93,    29,    30,
      40,    32,    33,    34,   103,    36,    37,    38,    92,   103,
      68,   144,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     5,    40,     5,
      93,    39,    10,    68,    39,   143,    14,   102,     1,    17,
      18,   144,    98,   293,   350,    55,    24,    61,    26,   136,
     114,    29,    30,   119,    32,   252,    34,   272,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      -1,    -1,    81,    82,    -1,    -1,    85,    -1,    -1,    -1,
      89,    90,    91
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   164,     0,     5,    10,    11,    13,    14,    15,    16,
      17,    18,    19,    24,    25,    26,    29,    30,    32,    33,
      34,    36,    37,    38,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,   165,
     171,   178,   189,   190,   191,   194,   195,   196,   197,   198,
     199,   200,   201,    40,    39,    16,    40,    40,     3,    39,
      42,    40,    13,    16,    99,   166,   167,   143,   174,    99,
      39,   192,   193,    14,   195,   201,    68,     3,    40,    99,
      68,    97,   103,   187,    20,    22,    23,   169,   170,   172,
      39,   175,    92,   141,    68,   104,    39,    40,    39,     4,
      39,    69,   171,   187,   188,   189,   204,   205,   207,   209,
      40,    40,    40,   168,   169,    24,    26,   181,   176,    40,
      93,    40,   193,    40,    41,    42,   103,   202,   225,    68,
      68,   208,    98,    39,   100,   102,   187,   189,   204,    99,
      39,    92,    95,    96,   100,   103,   143,   217,   218,   219,
     220,   222,   223,   225,   205,    68,   187,   182,   183,   144,
     143,    93,   142,   202,   203,   225,    40,    39,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    94,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   198,
     199,   210,   211,   212,   213,   214,   215,   216,    71,    72,
      73,    74,    76,    81,    82,    85,    89,    90,    91,    39,
      77,    79,    92,    97,   101,   106,    39,    40,   226,   107,
      39,    92,    96,   219,   223,    39,    39,   218,    68,    99,
      40,   191,   191,   173,   177,   102,    68,   102,   210,    39,
      40,   224,   225,   226,    40,    39,    39,    68,    77,    79,
      97,    93,    77,    79,    39,    39,    68,   102,   144,   143,
     217,   218,    68,    12,   184,   193,   175,   179,   181,   225,
      97,    93,    93,    93,    77,    79,    77,    79,   221,    40,
      39,   206,   144,   217,    68,    40,     5,    17,    18,    29,
     185,   186,   193,    68,   144,    39,   104,   219,    68,   102,
      68,   144,    40,   186,   180,    77,    79,    39,    40,    93,
      39,   218,   181,    77,    79,    68,   102,    68,    39,   143,
     102,   217,   144
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 6:

/* Line 1806 of yacc.c  */
#line 221 "ptx.y"
    { set_symtab((yyvsp[(1) - (1)].ptr_value)); func_header(".skip"); }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 221 "ptx.y"
    { end_function(); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 222 "ptx.y"
    { set_symtab((yyvsp[(1) - (1)].ptr_value)); }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 222 "ptx.y"
    { func_header(".skip"); }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 222 "ptx.y"
    { end_function(); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 225 "ptx.y"
    {func_header_info_int(".maxntid", (yyvsp[(2) - (6)].int_value));
										func_header_info_int(",", (yyvsp[(4) - (6)].int_value));
										func_header_info_int(",", (yyvsp[(6) - (6)].int_value)); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 228 "ptx.y"
    { func_header_info_int(".minnctapersm", (yyvsp[(2) - (2)].int_value)); printf("GPGPU-Sim: Warning: .minnctapersm ignored. \n"); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 229 "ptx.y"
    { func_header_info_int(".maxnctapersm", (yyvsp[(2) - (2)].int_value)); printf("GPGPU-Sim: Warning: .maxnctapersm ignored. \n"); }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 236 "ptx.y"
    { start_function((yyvsp[(1) - (2)].int_value)); func_header_info("(");}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 236 "ptx.y"
    {func_header_info(")");}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 236 "ptx.y"
    { (yyval.ptr_value) = reset_symtab(); }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 237 "ptx.y"
    { start_function((yyvsp[(1) - (1)].int_value)); }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 237 "ptx.y"
    { (yyval.ptr_value) = reset_symtab(); }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 238 "ptx.y"
    { start_function((yyvsp[(1) - (1)].int_value)); add_function_name(""); g_func_decl=0; (yyval.ptr_value) = reset_symtab(); }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 241 "ptx.y"
    { add_function_name((yyvsp[(1) - (1)].string_value)); }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 241 "ptx.y"
    {func_header_info("(");}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 241 "ptx.y"
    { g_func_decl=0; func_header_info(")"); }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 242 "ptx.y"
    { add_function_name((yyvsp[(1) - (1)].string_value)); g_func_decl=0; }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 245 "ptx.y"
    { (yyval.int_value) = 1; g_func_decl=1; func_header(".entry"); }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 246 "ptx.y"
    { (yyval.int_value) = 1; g_func_decl=1; func_header(".entry"); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 247 "ptx.y"
    { (yyval.int_value) = 0; g_func_decl=1; func_header(".func"); }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 248 "ptx.y"
    { (yyval.int_value) = 0; g_func_decl=1; func_header(".func"); }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 249 "ptx.y"
    { (yyval.int_value) = 2; g_func_decl=1; func_header(".func"); }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 253 "ptx.y"
    { add_directive(); }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 254 "ptx.y"
    {func_header_info(",");}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 254 "ptx.y"
    { add_directive(); }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 256 "ptx.y"
    { add_space_spec(param_space_unclassified,0); }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 256 "ptx.y"
    { add_function_arg(); }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 257 "ptx.y"
    { add_space_spec(reg_space,0); }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 257 "ptx.y"
    { add_function_arg(); }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 263 "ptx.y"
    { add_ptr_spec(global_space); }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 264 "ptx.y"
    { add_ptr_spec(local_space); }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 265 "ptx.y"
    { add_ptr_spec(shared_space); }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 271 "ptx.y"
    { add_directive(); }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 272 "ptx.y"
    { add_instruction(); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 273 "ptx.y"
    { add_directive(); }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 274 "ptx.y"
    { add_instruction(); }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 280 "ptx.y"
    { add_version_info((yyvsp[(2) - (2)].double_value), 0); }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 281 "ptx.y"
    { add_version_info((yyvsp[(2) - (3)].double_value),1); }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 282 "ptx.y"
    {/*Do nothing*/}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 283 "ptx.y"
    { target_header2((yyvsp[(2) - (4)].string_value),(yyvsp[(4) - (4)].string_value)); }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 284 "ptx.y"
    { target_header3((yyvsp[(2) - (6)].string_value),(yyvsp[(4) - (6)].string_value),(yyvsp[(6) - (6)].string_value)); }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 285 "ptx.y"
    { target_header((yyvsp[(2) - (2)].string_value)); }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 286 "ptx.y"
    { add_file((yyvsp[(2) - (3)].int_value),(yyvsp[(3) - (3)].string_value)); }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 288 "ptx.y"
    { add_pragma((yyvsp[(2) - (3)].string_value)); }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 289 "ptx.y"
    {/*Do nothing*/}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 292 "ptx.y"
    { add_variables(); }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 293 "ptx.y"
    { add_variables(); }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 294 "ptx.y"
    { add_variables(); }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 295 "ptx.y"
    { add_constptr((yyvsp[(2) - (6)].string_value), (yyvsp[(4) - (6)].string_value), (yyvsp[(6) - (6)].int_value)); }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 298 "ptx.y"
    { set_variable_type(); }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 303 "ptx.y"
    { add_identifier((yyvsp[(1) - (1)].string_value),0,NON_ARRAY_IDENTIFIER); func_header_info((yyvsp[(1) - (1)].string_value));}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 304 "ptx.y"
    { func_header_info((yyvsp[(1) - (4)].string_value)); func_header_info_int("<", (yyvsp[(3) - (4)].int_value)); func_header_info(">");
		int i,lbase,l;
		char *id = NULL;
		lbase = strlen((yyvsp[(1) - (4)].string_value));
		for( i=0; i < (yyvsp[(3) - (4)].int_value); i++ ) { 
			l = lbase + (int)log10(i+1)+10;
			id = (char*) malloc(l);
			snprintf(id,l,"%s%u",(yyvsp[(1) - (4)].string_value),i);
			add_identifier(id,0,NON_ARRAY_IDENTIFIER); 
		}
		free((yyvsp[(1) - (4)].string_value));
	}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 316 "ptx.y"
    { add_identifier((yyvsp[(1) - (3)].string_value),0,ARRAY_IDENTIFIER_NO_DIM); func_header_info((yyvsp[(1) - (3)].string_value)); func_header_info("["); func_header_info("]");}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 317 "ptx.y"
    { add_identifier((yyvsp[(1) - (4)].string_value),(yyvsp[(3) - (4)].int_value),ARRAY_IDENTIFIER); func_header_info((yyvsp[(1) - (4)].string_value)); func_header_info_int("[",(yyvsp[(3) - (4)].int_value)); func_header_info("]");}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 326 "ptx.y"
    { add_extern_spec(); }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 329 "ptx.y"
    { add_alignment_spec((yyvsp[(2) - (2)].int_value)); }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 331 "ptx.y"
    {  add_space_spec(reg_space,0); }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 332 "ptx.y"
    {  add_space_spec(reg_space,0); }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 336 "ptx.y"
    {  add_space_spec(const_space,(yyvsp[(1) - (1)].int_value)); }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 337 "ptx.y"
    {  add_space_spec(global_space,0); }
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 338 "ptx.y"
    {  add_space_spec(local_space,0); }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 339 "ptx.y"
    {  add_space_spec(param_space_unclassified,0); }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 340 "ptx.y"
    {  add_space_spec(shared_space,0); }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 341 "ptx.y"
    {  add_space_spec(surf_space,0); }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 342 "ptx.y"
    {  add_space_spec(tex_space,0); }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 349 "ptx.y"
    {  add_option(V2_TYPE); func_header_info(".v2");}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 350 "ptx.y"
    {  add_option(V3_TYPE); func_header_info(".v3");}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 351 "ptx.y"
    {  add_option(V4_TYPE); func_header_info(".v4");}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 354 "ptx.y"
    { add_scalar_type_spec( S8_TYPE ); }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 355 "ptx.y"
    { add_scalar_type_spec( S16_TYPE ); }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 356 "ptx.y"
    { add_scalar_type_spec( S32_TYPE ); }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 357 "ptx.y"
    { add_scalar_type_spec( S64_TYPE ); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 358 "ptx.y"
    { add_scalar_type_spec( U8_TYPE ); }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 359 "ptx.y"
    { add_scalar_type_spec( U16_TYPE ); }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 360 "ptx.y"
    { add_scalar_type_spec( U32_TYPE ); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 361 "ptx.y"
    { add_scalar_type_spec( U64_TYPE ); }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 362 "ptx.y"
    { add_scalar_type_spec( F16_TYPE ); }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 363 "ptx.y"
    { add_scalar_type_spec( F32_TYPE ); }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 364 "ptx.y"
    { add_scalar_type_spec( F64_TYPE ); }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 365 "ptx.y"
    { add_scalar_type_spec( FF64_TYPE ); }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 366 "ptx.y"
    { add_scalar_type_spec( B8_TYPE );  }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 367 "ptx.y"
    { add_scalar_type_spec( B16_TYPE ); }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 368 "ptx.y"
    { add_scalar_type_spec( B32_TYPE ); }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 369 "ptx.y"
    { add_scalar_type_spec( B64_TYPE ); }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 370 "ptx.y"
    { add_scalar_type_spec( BB64_TYPE ); }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 371 "ptx.y"
    { add_scalar_type_spec( BB128_TYPE ); }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 372 "ptx.y"
    { add_scalar_type_spec( PRED_TYPE ); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 373 "ptx.y"
    { add_scalar_type_spec( TEXREF_TYPE ); }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 374 "ptx.y"
    { add_scalar_type_spec( SAMPLERREF_TYPE ); }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 375 "ptx.y"
    { add_scalar_type_spec( SURFREF_TYPE ); }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 378 "ptx.y"
    { add_array_initializer(); }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 379 "ptx.y"
    { syntax_not_implemented(); }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 385 "ptx.y"
    { add_label((yyvsp[(1) - (2)].string_value)); }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 388 "ptx.y"
    { set_return(); }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 395 "ptx.y"
    { add_opcode((yyvsp[(1) - (1)].int_value)); }
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 396 "ptx.y"
    { add_opcode((yyvsp[(1) - (1)].int_value)); }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 398 "ptx.y"
    { add_pred((yyvsp[(2) - (2)].string_value),0, -1); }
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 399 "ptx.y"
    { add_pred((yyvsp[(3) - (3)].string_value),1, -1); }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 400 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,1); }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 401 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,2); }
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 402 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,3); }
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 403 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,5); }
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 404 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,6); }
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 405 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,10); }
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 406 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,12); }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 407 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,13); }
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 408 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,17); }
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 409 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,19); }
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 410 "ptx.y"
    { add_pred((yyvsp[(2) - (3)].string_value),0,28); }
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 420 "ptx.y"
    { add_option(SYNC_OPTION); }
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 421 "ptx.y"
    { add_option(ARRIVE_OPTION); }
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 422 "ptx.y"
    { add_option(RED_OPTION); }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 423 "ptx.y"
    { add_option(POPC_REDUCTION); }
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 424 "ptx.y"
    { add_option(AND_REDUCTION); }
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 425 "ptx.y"
    { add_option(OR_REDUCTION); }
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 426 "ptx.y"
    { add_option(UNI_OPTION); }
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 427 "ptx.y"
    { add_option(WIDE_OPTION); }
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 428 "ptx.y"
    { add_option(ANY_OPTION); }
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 429 "ptx.y"
    { add_option(ALL_OPTION); }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 430 "ptx.y"
    { add_option(BALLOT_OPTION); }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 431 "ptx.y"
    { add_option(GLOBAL_OPTION); }
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 432 "ptx.y"
    { add_option(CTA_OPTION); }
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 433 "ptx.y"
    { add_option(SYS_OPTION); }
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 434 "ptx.y"
    { add_option(GEOM_MODIFIER_1D); }
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 435 "ptx.y"
    { add_option(GEOM_MODIFIER_2D); }
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 436 "ptx.y"
    { add_option(GEOM_MODIFIER_3D); }
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 437 "ptx.y"
    { add_option(SAT_OPTION); }
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 438 "ptx.y"
    { add_option(FTZ_OPTION); }
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 439 "ptx.y"
    { add_option(NEG_OPTION); }
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 440 "ptx.y"
    { add_option(APPROX_OPTION); }
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 441 "ptx.y"
    { add_option(FULL_OPTION); }
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 442 "ptx.y"
    { add_option(EXIT_OPTION); }
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 443 "ptx.y"
    { add_option(ABS_OPTION); }
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 445 "ptx.y"
    { add_option(TO_OPTION); }
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 446 "ptx.y"
    { add_option(HALF_OPTION); }
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 447 "ptx.y"
    { add_option(CA_OPTION); }
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 448 "ptx.y"
    { add_option(CG_OPTION); }
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 449 "ptx.y"
    { add_option(CS_OPTION); }
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 450 "ptx.y"
    { add_option(LU_OPTION); }
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 451 "ptx.y"
    { add_option(CV_OPTION); }
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 452 "ptx.y"
    { add_option(WB_OPTION); }
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 453 "ptx.y"
    { add_option(WT_OPTION); }
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 456 "ptx.y"
    { add_option(ATOMIC_AND); }
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 457 "ptx.y"
    { add_option(ATOMIC_OR); }
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 458 "ptx.y"
    { add_option(ATOMIC_XOR); }
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 459 "ptx.y"
    { add_option(ATOMIC_CAS); }
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 460 "ptx.y"
    { add_option(ATOMIC_EXCH); }
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 461 "ptx.y"
    { add_option(ATOMIC_CAS_SYNC); }
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 462 "ptx.y"
    { add_option(ATOMIC_EXCH_SYNC); }
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 463 "ptx.y"
    { add_option(ATOMIC_ADD); }
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 464 "ptx.y"
    { add_option(ATOMIC_INC); }
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 465 "ptx.y"
    { add_option(ATOMIC_DEC); }
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 466 "ptx.y"
    { add_option(ATOMIC_MIN); }
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 467 "ptx.y"
    { add_option(ATOMIC_MAX); }
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 473 "ptx.y"
    { add_option(RN_OPTION); }
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 474 "ptx.y"
    { add_option(RZ_OPTION); }
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 475 "ptx.y"
    { add_option(RM_OPTION); }
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 476 "ptx.y"
    { add_option(RP_OPTION); }
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 479 "ptx.y"
    { add_option(RNI_OPTION); }
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 480 "ptx.y"
    { add_option(RZI_OPTION); }
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 481 "ptx.y"
    { add_option(RMI_OPTION); }
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 482 "ptx.y"
    { add_option(RPI_OPTION); }
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 485 "ptx.y"
    { add_option(EQ_OPTION); }
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 486 "ptx.y"
    { add_option(NE_OPTION); }
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 487 "ptx.y"
    { add_option(LT_OPTION); }
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 488 "ptx.y"
    { add_option(LE_OPTION); }
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 489 "ptx.y"
    { add_option(GT_OPTION); }
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 490 "ptx.y"
    { add_option(GE_OPTION); }
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 491 "ptx.y"
    { add_option(LO_OPTION); }
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 492 "ptx.y"
    { add_option(LS_OPTION); }
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 493 "ptx.y"
    { add_option(HI_OPTION); }
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 494 "ptx.y"
    { add_option(HS_OPTION); }
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 495 "ptx.y"
    { add_option(EQU_OPTION); }
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 496 "ptx.y"
    { add_option(NEU_OPTION); }
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 497 "ptx.y"
    { add_option(LTU_OPTION); }
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 498 "ptx.y"
    { add_option(LEU_OPTION); }
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 499 "ptx.y"
    { add_option(GTU_OPTION); }
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 500 "ptx.y"
    { add_option(GEU_OPTION); }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 501 "ptx.y"
    { add_option(NUM_OPTION); }
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 502 "ptx.y"
    { add_option(NAN_OPTION); }
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 508 "ptx.y"
    { add_scalar_operand( (yyvsp[(1) - (1)].string_value) ); }
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 509 "ptx.y"
    { add_neg_pred_operand( (yyvsp[(2) - (2)].string_value) ); }
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 510 "ptx.y"
    { add_scalar_operand( (yyvsp[(2) - (2)].string_value) ); change_operand_neg(); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 515 "ptx.y"
    { change_operand_neg(); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 517 "ptx.y"
    { add_address_operand((yyvsp[(1) - (3)].string_value),(yyvsp[(3) - (3)].int_value)); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 518 "ptx.y"
    { add_scalar_operand( (yyvsp[(1) - (2)].string_value) ); change_operand_lohi(1);}
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 519 "ptx.y"
    { add_scalar_operand( (yyvsp[(2) - (3)].string_value) ); change_operand_lohi(1); change_operand_neg();}
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 520 "ptx.y"
    { add_scalar_operand( (yyvsp[(1) - (2)].string_value) ); change_operand_lohi(2);}
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 521 "ptx.y"
    { add_scalar_operand( (yyvsp[(2) - (3)].string_value) ); change_operand_lohi(2); change_operand_neg();}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 522 "ptx.y"
    { add_2vector_operand((yyvsp[(1) - (3)].string_value),(yyvsp[(3) - (3)].string_value)); change_double_operand_type(-1);}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 523 "ptx.y"
    { add_2vector_operand((yyvsp[(1) - (4)].string_value),(yyvsp[(3) - (4)].string_value)); change_double_operand_type(-1); change_operand_lohi(1);}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 524 "ptx.y"
    { add_2vector_operand((yyvsp[(1) - (4)].string_value),(yyvsp[(3) - (4)].string_value)); change_double_operand_type(-1); change_operand_lohi(2);}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 525 "ptx.y"
    { add_2vector_operand((yyvsp[(1) - (3)].string_value),(yyvsp[(3) - (3)].string_value)); change_double_operand_type(-3);}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 526 "ptx.y"
    { add_2vector_operand((yyvsp[(1) - (4)].string_value),(yyvsp[(3) - (4)].string_value)); change_double_operand_type(-3); change_operand_lohi(1);}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 527 "ptx.y"
    { add_2vector_operand((yyvsp[(1) - (4)].string_value),(yyvsp[(3) - (4)].string_value)); change_double_operand_type(-3); change_operand_lohi(2);}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 530 "ptx.y"
    { add_2vector_operand((yyvsp[(2) - (5)].string_value),(yyvsp[(4) - (5)].string_value)); }
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 531 "ptx.y"
    { add_3vector_operand((yyvsp[(2) - (7)].string_value),(yyvsp[(4) - (7)].string_value),(yyvsp[(6) - (7)].string_value)); }
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 532 "ptx.y"
    { add_4vector_operand((yyvsp[(2) - (9)].string_value),(yyvsp[(4) - (9)].string_value),(yyvsp[(6) - (9)].string_value),(yyvsp[(8) - (9)].string_value)); }
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 533 "ptx.y"
    { add_1vector_operand((yyvsp[(2) - (3)].string_value)); }
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 536 "ptx.y"
    { add_scalar_operand((yyvsp[(2) - (3)].string_value)); }
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 541 "ptx.y"
    { add_builtin_operand((yyvsp[(1) - (2)].int_value),(yyvsp[(2) - (2)].int_value)); }
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 542 "ptx.y"
    { add_builtin_operand((yyvsp[(1) - (1)].int_value),-1); }
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 545 "ptx.y"
    { add_memory_operand(); }
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 546 "ptx.y"
    { add_memory_operand(); change_memory_addr_space((yyvsp[(1) - (4)].string_value)); }
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 547 "ptx.y"
    { change_memory_addr_space((yyvsp[(1) - (4)].string_value)); }
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 548 "ptx.y"
    { change_memory_addr_space((yyvsp[(1) - (4)].string_value)); add_memory_operand();}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 549 "ptx.y"
    { change_operand_neg(); }
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 552 "ptx.y"
    { add_double_operand((yyvsp[(1) - (3)].string_value),(yyvsp[(3) - (3)].string_value)); change_double_operand_type(1); }
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 553 "ptx.y"
    { add_double_operand((yyvsp[(1) - (4)].string_value),(yyvsp[(3) - (4)].string_value)); change_double_operand_type(1); change_operand_lohi(1); }
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 554 "ptx.y"
    { add_double_operand((yyvsp[(1) - (4)].string_value),(yyvsp[(3) - (4)].string_value)); change_double_operand_type(1); change_operand_lohi(2); }
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 555 "ptx.y"
    { add_double_operand((yyvsp[(1) - (4)].string_value),(yyvsp[(4) - (4)].string_value)); change_double_operand_type(2); }
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 556 "ptx.y"
    { add_double_operand((yyvsp[(1) - (5)].string_value),(yyvsp[(4) - (5)].string_value)); change_double_operand_type(2); change_operand_lohi(1); }
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 557 "ptx.y"
    { add_double_operand((yyvsp[(1) - (5)].string_value),(yyvsp[(4) - (5)].string_value)); change_double_operand_type(2); change_operand_lohi(2); }
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 558 "ptx.y"
    { add_address_operand((yyvsp[(1) - (4)].string_value),(yyvsp[(4) - (4)].int_value)); change_double_operand_type(3); }
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 561 "ptx.y"
    { add_literal_int((yyvsp[(1) - (1)].int_value)); }
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 562 "ptx.y"
    { add_literal_float((yyvsp[(1) - (1)].float_value)); }
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 563 "ptx.y"
    { add_literal_double((yyvsp[(1) - (1)].double_value)); }
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 566 "ptx.y"
    { add_address_operand((yyvsp[(1) - (1)].string_value),0); }
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 567 "ptx.y"
    { add_address_operand((yyvsp[(1) - (2)].string_value),0); change_operand_lohi(1);}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 568 "ptx.y"
    { add_address_operand((yyvsp[(1) - (2)].string_value),0); change_operand_lohi(2); }
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 569 "ptx.y"
    { add_address_operand((yyvsp[(1) - (3)].string_value),(yyvsp[(3) - (3)].int_value)); }
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 570 "ptx.y"
    { add_address_operand2((yyvsp[(1) - (1)].int_value)); }
    break;



/* Line 1806 of yacc.c  */
#line 3606 "/home/ashafiey/workspace/depot/gpgpu_sim_research/mimdlike/distribution/build/gcc-4.6.2/cuda-4020/debug/cuda-sim/ptx.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 573 "ptx.y"


extern int ptx_lineno;
extern const char *g_filename;

void syntax_not_implemented()
{
	printf("Parse error (%s:%u): this syntax is not (yet) implemented:\n",g_filename,ptx_lineno);
	ptx_error(NULL);
	abort();
}

