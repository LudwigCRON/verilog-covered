#ifndef __GENPROF_H__
#define __GENPROF_H__

/*
 Copyright (c) 2006 Trevor Williams

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program;
 if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*!
 \file    genprof.h
 \author  Trevor Williams  (phase1geo@gmail.com)
 \date    12/10/2007
*/

#include "defines.h"

#define NUM_PROFILES 1043

#ifdef DEBUG
#define UNREGISTERED 0
#define FSM_ARG_PARSE_STATE 1
#define FSM_ARG_PARSE 2
#define FSM_ARG_PARSE_VALUE 3
#define FSM_ARG_PARSE_TRANS 4
#define FSM_ARG_PARSE_ATTR 5
#define SYM_VALUE_STORE 6
#define ADD_SYM_VALUES_TO_SIM 7
#define COVERED_VALUE_CHANGE_BIN 8
#define COVERED_VALUE_CHANGE_REAL 9
#define COVERED_END_OF_SIM 10
#define COVERED_CB_ERROR_HANDLER 11
#define GEN_NEXT_SYMBOL 12
#define COVERED_CREATE_VALUE_CHANGE_CB 13
#define COVERED_PARSE_TASK_FUNC 14
#define COVERED_PARSE_SIGNALS 15
#define COVERED_PARSE_INSTANCE 16
#define COVERED_SIM_CALLTF 17
#define COVERED_REGISTER 18
#define LEXER_KEYWORD_SV_CODE 19
#define VSIGNAL_INIT 20
#define VSIGNAL_CREATE 21
#define VSIGNAL_CREATE_VEC 22
#define VSIGNAL_DUPLICATE 23
#define VSIGNAL_DB_WRITE 24
#define VSIGNAL_DB_READ 25
#define VSIGNAL_DB_MERGE 26
#define VSIGNAL_MERGE 27
#define VSIGNAL_PROPAGATE 28
#define VSIGNAL_VCD_ASSIGN 29
#define VSIGNAL_ADD_EXPRESSION 30
#define VSIGNAL_FROM_STRING 31
#define VSIGNAL_CALC_WIDTH_FOR_EXPR 32
#define VSIGNAL_CALC_LSB_FOR_EXPR 33
#define VSIGNAL_DEALLOC 34
#define LEXER_KEYWORD_1995_CODE 35
#define STMT_ITER_RESET 36
#define STMT_ITER_COPY 37
#define STMT_ITER_NEXT 38
#define STMT_ITER_REVERSE 39
#define STMT_ITER_FIND_HEAD 40
#define STMT_ITER_GET_NEXT_IN_ORDER 41
#define STMT_ITER_GET_LINE_BEFORE 42
#define LEXER_CSTRING_A 43
#define LEXER_KEYWORD_A 44
#define LEXER_ESCAPED_NAME 45
#define LEXER_SYSTEM_CALL 46
#define LINE_DIRECTIVE 47
#define LINE_DIRECTIVE2 48
#define PROCESS_TIMESCALE_TOKEN 49
#define PROCESS_TIMESCALE 50
#define LEXER_YYWRAP 51
#define RESET_LEXER 52
#define CHECK_FOR_PRAGMA 53
#define STATEMENT_CREATE 54
#define STATEMENT_QUEUE_ADD 55
#define STATEMENT_QUEUE_COMPARE 56
#define STATEMENT_SIZE_ELEMENTS 57
#define STATEMENT_DB_WRITE 58
#define STATEMENT_DB_WRITE_TREE 59
#define STATEMENT_DB_WRITE_EXPR_TREE 60
#define STATEMENT_DB_READ 61
#define STATEMENT_ASSIGN_EXPR_IDS 62
#define STATEMENT_CONNECT 63
#define STATEMENT_GET_LAST_LINE_HELPER 64
#define STATEMENT_GET_LAST_LINE 65
#define STATEMENT_FIND_RHS_SIGS 66
#define STATEMENT_FIND_HEAD_STATEMENT 67
#define STATEMENT_FIND_STATEMENT 68
#define STATEMENT_CONTAINS_EXPR_CALLING_STMT 69
#define STATEMENT_DEALLOC_RECURSIVE 70
#define STATEMENT_DEALLOC 71
#define ENUMERATE_ADD_ITEM 72
#define ENUMERATE_END_LIST 73
#define ENUMERATE_RESOLVE 74
#define ENUMERATE_DEALLOC 75
#define ENUMERATE_DEALLOC_LIST 76
#define PARSER_PORT_DECLARATION_A 77
#define PARSER_PORT_DECLARATION_B 78
#define PARSER_PORT_DECLARATION_C 79
#define PARSER_STATIC_EXPR_PRIMARY_A 80
#define PARSER_STATIC_EXPR_PRIMARY_B 81
#define PARSER_EXPRESSION_LIST_A 82
#define PARSER_EXPRESSION_LIST_B 83
#define PARSER_EXPRESSION_LIST_C 84
#define PARSER_EXPRESSION_LIST_D 85
#define PARSER_IDENTIFIER_A 86
#define PARSER_GENERATE_CASE_ITEM_A 87
#define PARSER_GENERATE_CASE_ITEM_B 88
#define PARSER_GENERATE_CASE_ITEM_C 89
#define PARSER_STATEMENT_BEGIN_A 90
#define PARSER_STATEMENT_FORK_A 91
#define PARSER_STATEMENT_FOR_A 92
#define PARSER_CASE_ITEM_A 93
#define PARSER_CASE_ITEM_B 94
#define PARSER_CASE_ITEM_C 95
#define PARSER_DELAY_VALUE_A 96
#define PARSER_DELAY_VALUE_B 97
#define PARSER_PARAMETER_VALUE_BYNAME_A 98
#define PARSER_GATE_INSTANCE_A 99
#define PARSER_GATE_INSTANCE_B 100
#define PARSER_GATE_INSTANCE_C 101
#define PARSER_GATE_INSTANCE_D 102
#define PARSER_LIST_OF_NAMES_A 103
#define PARSER_LIST_OF_NAMES_B 104
#define ATTRIBUTE_CREATE 105
#define ATTRIBUTE_PARSE 106
#define ATTRIBUTE_DEALLOC 107
#define SEARCH_INIT 108
#define SEARCH_ADD_INCLUDE_PATH 109
#define SEARCH_ADD_DIRECTORY_PATH 110
#define SEARCH_ADD_FILE 111
#define SEARCH_ADD_NO_SCORE_FUNIT 112
#define SEARCH_ADD_EXTENSIONS 113
#define SEARCH_FREE_LISTS 114
#define RANK_CREATE_COMP_CDD_COV 115
#define RANK_DEALLOC_COMP_CDD_COV 116
#define RANK_CHECK_INDEX 117
#define RANK_GATHER_SIGNAL_COV 118
#define RANK_GATHER_COMB_COV 119
#define RANK_GATHER_EXPRESSION_COV 120
#define RANK_GATHER_FSM_COV 121
#define RANK_CALC_NUM_CPS 122
#define RANK_GATHER_COMP_CDD_COV 123
#define RANK_READ_CDD 124
#define RANK_SELECTED_CDD_COV 125
#define RANK_PERFORM_WEIGHTED_SELECTION 126
#define RANK_PERFORM_GREEDY_SORT 127
#define RANK_COUNT_CPS 128
#define RANK_PERFORM 129
#define RANK_OUTPUT 130
#define COMMAND_RANK 131
#define SIM_CURRENT_THREAD 132
#define SIM_THREAD_POP_HEAD 133
#define SIM_THREAD_INSERT_INTO_DELAY_QUEUE 134
#define SIM_THREAD_PUSH 135
#define SIM_EXPR_CHANGED 136
#define SIM_CREATE_THREAD 137
#define SIM_ADD_THREAD 138
#define SIM_KILL_THREAD 139
#define SIM_KILL_THREAD_WITH_FUNIT 140
#define SIM_ADD_STATICS 141
#define SIM_EXPRESSION 142
#define SIM_THREAD 143
#define SIM_SIMULATE 144
#define SIM_INITIALIZE 145
#define SIM_STOP 146
#define SIM_FINISH 147
#define SIM_DEALLOC 148
#define SCOPE_FIND_FUNIT_FROM_SCOPE 149
#define SCOPE_FIND_PARAM 150
#define SCOPE_FIND_SIGNAL 151
#define SCOPE_FIND_TASK_FUNCTION_NAMEDBLOCK 152
#define SCOPE_GET_PARENT_FUNIT 153
#define SCOPE_GET_PARENT_MODULE 154
#define REPORT_PARSE_METRICS 155
#define REPORT_PARSE_ARGS 156
#define REPORT_GATHER_INSTANCE_STATS 157
#define REPORT_GATHER_FUNIT_STATS 158
#define REPORT_PRINT_HEADER 159
#define REPORT_GENERATE 160
#define REPORT_READ_CDD_AND_READY 161
#define REPORT_CLOSE_CDD 162
#define REPORT_SAVE_CDD 163
#define REPORT_FORMAT_EXCLUSION_REASON 164
#define REPORT_OUTPUT_EXCLUSION_REASON 165
#define COMMAND_REPORT 166
#define CHECK_OPTION_VALUE 167
#define IS_VARIABLE 168
#define IS_FUNC_UNIT 169
#define IS_LEGAL_FILENAME 170
#define GET_BASENAME 171
#define GET_DIRNAME 172
#define GET_ABSOLUTE_PATH 173
#define GET_RELATIVE_PATH 174
#define DIRECTORY_EXISTS 175
#define DIRECTORY_LOAD 176
#define FILE_EXISTS 177
#define UTIL_READLINE 178
#define GET_QUOTED_STRING 179
#define SUBSTITUTE_ENV_VARS 180
#define SCOPE_EXTRACT_FRONT 181
#define SCOPE_EXTRACT_BACK 182
#define SCOPE_EXTRACT_SCOPE 183
#define SCOPE_GEN_PRINTABLE 184
#define SCOPE_COMPARE 185
#define SCOPE_LOCAL 186
#define CONVERT_FILE_TO_MODULE 187
#define GET_NEXT_VFILE 188
#define GEN_SPACE 189
#define REMOVE_UNDERSCORES 190
#define GET_FUNIT_TYPE 191
#define CALC_MISS_PERCENT 192
#define READ_COMMAND_FILE 193
#define OVL_IS_ASSERTION_NAME 194
#define OVL_IS_ASSERTION_MODULE 195
#define OVL_IS_COVERAGE_POINT 196
#define OVL_ADD_ASSERTIONS_TO_NO_SCORE_LIST 197
#define OVL_GET_FUNIT_STATS 198
#define OVL_GET_COVERAGE_POINT 199
#define OVL_DISPLAY_VERBOSE 200
#define OVL_COLLECT 201
#define OVL_GET_COVERAGE 202
#define COMBINATION_CALC_DEPTH 203
#define COMBINATION_DOES_MULTI_EXP_NEED_UL 204
#define COMBINATION_MULTI_EXPR_CALC 205
#define COMBINATION_IS_EXPR_MULTI_NODE 206
#define COMBINATION_GET_TREE_STATS 207
#define COMBINATION_RESET_COUNTED_EXPRS 208
#define COMBINATION_RESET_COUNTED_EXPR_TREE 209
#define COMBINATION_GET_STATS 210
#define COMBINATION_GET_FUNIT_SUMMARY 211
#define COMBINATION_GET_INST_SUMMARY 212
#define COMBINATION_DISPLAY_INSTANCE_SUMMARY 213
#define COMBINATION_INSTANCE_SUMMARY 214
#define COMBINATION_DISPLAY_FUNIT_SUMMARY 215
#define COMBINATION_FUNIT_SUMMARY 216
#define COMBINATION_DRAW_LINE 217
#define COMBINATION_DRAW_CENTERED_LINE 218
#define COMBINATION_UNDERLINE_TREE 219
#define COMBINATION_PREP_LINE 220
#define COMBINATION_UNDERLINE 221
#define COMBINATION_UNARY 222
#define COMBINATION_EVENT 223
#define COMBINATION_TWO_VARS 224
#define COMBINATION_MULTI_VAR_EXPRS 225
#define COMBINATION_MULTI_EXPR_OUTPUT_LENGTH 226
#define COMBINATION_MULTI_EXPR_OUTPUT 227
#define COMBINATION_MULTI_VARS 228
#define COMBINATION_GET_MISSED_EXPR 229
#define COMBINATION_LIST_MISSED 230
#define COMBINATION_OUTPUT_EXPR 231
#define COMBINATION_DISPLAY_VERBOSE 232
#define COMBINATION_INSTANCE_VERBOSE 233
#define COMBINATION_FUNIT_VERBOSE 234
#define COMBINATION_COLLECT 235
#define COMBINATION_GET_EXCLUDE_LIST 236
#define COMBINATION_GET_EXPRESSION 237
#define COMBINATION_GET_COVERAGE 238
#define COMBINATION_REPORT 239
#define VCD_PARSE_DEF_IGNORE 240
#define VCD_PARSE_DEF_VAR 241
#define VCD_PARSE_DEF_SCOPE 242
#define VCD_PARSE_DEF 243
#define VCD_PARSE_SIM_VECTOR 244
#define VCD_PARSE_SIM_REAL 245
#define VCD_PARSE_SIM_IGNORE 246
#define VCD_PARSE_SIM 247
#define VCD_PARSE 248
#define STMT_BLK_ADD_TO_REMOVE_LIST 249
#define STMT_BLK_REMOVE 250
#define STMT_BLK_SPECIFY_REMOVAL_REASON 251
#define STR_LINK_ADD 252
#define STMT_LINK_ADD_HEAD 253
#define STMT_LINK_ADD_TAIL 254
#define STMT_LINK_MERGE 255
#define EXP_LINK_ADD 256
#define SIG_LINK_ADD 257
#define FSM_LINK_ADD 258
#define FUNIT_LINK_ADD 259
#define GITEM_LINK_ADD 260
#define INST_LINK_ADD 261
#define STR_LINK_FIND 262
#define STMT_LINK_FIND 263
#define EXP_LINK_FIND 264
#define SIG_LINK_FIND 265
#define FSM_LINK_FIND 266
#define FUNIT_LINK_FIND 267
#define GITEM_LINK_FIND 268
#define INST_LINK_FIND_BY_SCOPE 269
#define INST_LINK_FIND_BY_FUNIT 270
#define STR_LINK_REMOVE 271
#define EXP_LINK_REMOVE 272
#define GITEM_LINK_REMOVE 273
#define FUNIT_LINK_REMOVE 274
#define INST_LINK_FLATTEN 275
#define STR_LINK_DELETE_LIST 276
#define STMT_LINK_UNLINK 277
#define STMT_LINK_DELETE_LIST 278
#define EXP_LINK_DELETE_LIST 279
#define SIG_LINK_DELETE_LIST 280
#define FSM_LINK_DELETE_LIST 281
#define FUNIT_LINK_DELETE_LIST 282
#define GITEM_LINK_DELETE_LIST 283
#define INST_LINK_DELETE_LIST 284
#define LEXER_KEYWORD_SYS_SV_CODE 285
#define SCORE_GENERATE_TOP_VPI_MODULE 286
#define SCORE_GENERATE_TOP_DUMPVARS_MODULE 287
#define SCORE_GENERATE_PLI_TAB_FILE 288
#define SCORE_PARSE_DEFINE 289
#define SCORE_ADD_ARG 290
#define SCORE_PARSE_ARGS 291
#define COMMAND_SCORE 292
#define MEMORY_GET_STAT 293
#define MEMORY_GET_STATS 294
#define MEMORY_GET_FUNIT_SUMMARY 295
#define MEMORY_GET_INST_SUMMARY 296
#define MEMORY_CREATE_PDIM_BIT_ARRAY 297
#define MEMORY_GET_MEM_COVERAGE 298
#define MEMORY_GET_COVERAGE 299
#define MEMORY_COLLECT 300
#define MEMORY_DISPLAY_TOGGLE_INSTANCE_SUMMARY 301
#define MEMORY_TOGGLE_INSTANCE_SUMMARY 302
#define MEMORY_DISPLAY_AE_INSTANCE_SUMMARY 303
#define MEMORY_AE_INSTANCE_SUMMARY 304
#define MEMORY_DISPLAY_TOGGLE_FUNIT_SUMMARY 305
#define MEMORY_TOGGLE_FUNIT_SUMMARY 306
#define MEMORY_DISPLAY_AE_FUNIT_SUMMARY 307
#define MEMORY_AE_FUNIT_SUMMARY 308
#define MEMORY_DISPLAY_MEMORY 309
#define MEMORY_DISPLAY_VERBOSE 310
#define MEMORY_INSTANCE_VERBOSE 311
#define MEMORY_FUNIT_VERBOSE 312
#define MEMORY_REPORT 313
#define DB_CREATE 314
#define DB_CLOSE 315
#define DB_CHECK_FOR_TOP_MODULE 316
#define DB_WRITE 317
#define DB_READ 318
#define DB_MERGE_FUNITS 319
#define DB_SCALE_TO_PRECISION 320
#define DB_CREATE_UNNAMED_SCOPE 321
#define DB_IS_UNNAMED_SCOPE 322
#define DB_SET_TIMESCALE 323
#define DB_GET_CURR_FUNIT 324
#define DB_GET_EXCLUSION_ID_SIZE 325
#define DB_GEN_EXCLUSION_ID 326
#define DB_ADD_FILE_VERSION 327
#define DB_OUTPUT_DUMPVARS 328
#define DB_ADD_INSTANCE 329
#define DB_ADD_MODULE 330
#define DB_END_MODULE 331
#define DB_ADD_FUNCTION_TASK_NAMEDBLOCK 332
#define DB_END_FUNCTION_TASK_NAMEDBLOCK 333
#define DB_ADD_DECLARED_PARAM 334
#define DB_ADD_OVERRIDE_PARAM 335
#define DB_ADD_VECTOR_PARAM 336
#define DB_ADD_DEFPARAM 337
#define DB_ADD_SIGNAL 338
#define DB_ADD_ENUM 339
#define DB_END_ENUM_LIST 340
#define DB_ADD_TYPEDEF 341
#define DB_FIND_SIGNAL 342
#define DB_ADD_GEN_ITEM_BLOCK 343
#define DB_FIND_GEN_ITEM 344
#define DB_FIND_TYPEDEF 345
#define DB_GET_CURR_GEN_BLOCK 346
#define DB_CURR_SIGNAL_COUNT 347
#define DB_CREATE_EXPRESSION 348
#define DB_BIND_EXPR_TREE 349
#define DB_CREATE_EXPR_FROM_STATIC 350
#define DB_ADD_EXPRESSION 351
#define DB_CREATE_SENSITIVITY_LIST 352
#define DB_PARALLELIZE_STATEMENT 353
#define DB_CREATE_STATEMENT 354
#define DB_ADD_STATEMENT 355
#define DB_REMOVE_STATEMENT_FROM_CURRENT_FUNIT 356
#define DB_REMOVE_STATEMENT 357
#define DB_CONNECT_STATEMENT_TRUE 358
#define DB_CONNECT_STATEMENT_FALSE 359
#define DB_GEN_ITEM_CONNECT_TRUE 360
#define DB_GEN_ITEM_CONNECT_FALSE 361
#define DB_GEN_ITEM_CONNECT 362
#define DB_STATEMENT_CONNECT 363
#define DB_CREATE_ATTR_PARAM 364
#define DB_PARSE_ATTRIBUTE 365
#define DB_REMOVE_STMT_BLKS_CALLING_STATEMENT 366
#define DB_GEN_CURR_INST_SCOPE 367
#define DB_SYNC_CURR_INSTANCE 368
#define DB_SET_VCD_SCOPE 369
#define DB_VCD_UPSCOPE 370
#define DB_ASSIGN_SYMBOL 371
#define DB_SET_SYMBOL_CHAR 372
#define DB_SET_SYMBOL_STRING 373
#define DB_DO_TIMESTEP 374
#define FSM_CREATE 375
#define FSM_ADD_ARC 376
#define FSM_CREATE_TABLES 377
#define FSM_DB_WRITE 378
#define FSM_DB_READ 379
#define FSM_DB_MERGE 380
#define FSM_MERGE 381
#define FSM_TABLE_SET 382
#define FSM_GET_STATS 383
#define FSM_GET_FUNIT_SUMMARY 384
#define FSM_GET_INST_SUMMARY 385
#define FSM_GATHER_SIGNALS 386
#define FSM_COLLECT 387
#define FSM_GET_COVERAGE 388
#define FSM_DISPLAY_INSTANCE_SUMMARY 389
#define FSM_INSTANCE_SUMMARY 390
#define FSM_DISPLAY_FUNIT_SUMMARY 391
#define FSM_FUNIT_SUMMARY 392
#define FSM_DISPLAY_STATE_VERBOSE 393
#define FSM_DISPLAY_ARC_VERBOSE 394
#define FSM_DISPLAY_VERBOSE 395
#define FSM_INSTANCE_VERBOSE 396
#define FSM_FUNIT_VERBOSE 397
#define FSM_REPORT 398
#define FSM_DEALLOC 399
#define CODEGEN_CREATE_EXPR_HELPER 400
#define CODEGEN_CREATE_EXPR 401
#define CODEGEN_GEN_EXPR 402
#define TOGGLE_GET_STATS 403
#define TOGGLE_COLLECT 404
#define TOGGLE_GET_COVERAGE 405
#define TOGGLE_GET_FUNIT_SUMMARY 406
#define TOGGLE_GET_INST_SUMMARY 407
#define TOGGLE_DISPLAY_INSTANCE_SUMMARY 408
#define TOGGLE_INSTANCE_SUMMARY 409
#define TOGGLE_DISPLAY_FUNIT_SUMMARY 410
#define TOGGLE_FUNIT_SUMMARY 411
#define TOGGLE_DISPLAY_VERBOSE 412
#define TOGGLE_INSTANCE_VERBOSE 413
#define TOGGLE_FUNIT_VERBOSE 414
#define TOGGLE_REPORT 415
#define VLERROR 416
#define VLWARN 417
#define PARSER_DEALLOC_SIG_RANGE 418
#define PARSER_COPY_CURR_RANGE 419
#define PARSER_COPY_RANGE_TO_CURR_RANGE 420
#define PARSER_EXPLICITLY_SET_CURR_RANGE 421
#define PARSER_IMPLICITLY_SET_CURR_RANGE 422
#define PARSER_CHECK_GENERATION 423
#define LEXER_KEYWORD_2001_CODE 424
#define STRUCT_UNION_LENGTH 425
#define STRUCT_UNION_ADD_MEMBER 426
#define STRUCT_UNION_ADD_MEMBER_VOID 427
#define STRUCT_UNION_ADD_MEMBER_SIG 428
#define STRUCT_UNION_ADD_MEMBER_TYPEDEF 429
#define STRUCT_UNION_ADD_MEMBER_ENUM 430
#define STRUCT_UNION_ADD_MEMBER_STRUCT_UNION 431
#define STRUCT_UNION_CREATE 432
#define STRUCT_UNION_MEMBER_DEALLOC 433
#define STRUCT_UNION_DEALLOC 434
#define STRUCT_UNION_DEALLOC_LIST 435
#define BIND_ADD 436
#define BIND_APPEND_FSM_EXPR 437
#define BIND_REMOVE 438
#define BIND_FIND_SIG_NAME 439
#define BIND_PARAM 440
#define BIND_SIGNAL 441
#define BIND_TASK_FUNCTION_PORTS 442
#define BIND_TASK_FUNCTION_NAMEDBLOCK 443
#define BIND_PERFORM 444
#define BIND_DEALLOC 445
#define PERF_GEN_STATS 446
#define PERF_OUTPUT_MOD_STATS 447
#define PERF_OUTPUT_INST_REPORT_HELPER 448
#define PERF_OUTPUT_INST_REPORT 449
#define MOD_PARM_FIND 450
#define MOD_PARM_FIND_EXPR_AND_REMOVE 451
#define MOD_PARM_ADD 452
#define INST_PARM_FIND 453
#define INST_PARM_ADD 454
#define INST_PARM_ADD_GENVAR 455
#define INST_PARM_BIND 456
#define DEFPARAM_ADD 457
#define DEFPARAM_DEALLOC 458
#define PARAM_FIND_AND_SET_EXPR_VALUE 459
#define PARAM_SET_SIG_SIZE 460
#define PARAM_SIZE_FUNCTION 461
#define PARAM_EXPR_EVAL 462
#define PARAM_HAS_OVERRIDE 463
#define PARAM_HAS_DEFPARAM 464
#define PARAM_RESOLVE_DECLARED 465
#define PARAM_RESOLVE_OVERRIDE 466
#define PARAM_RESOLVE 467
#define PARAM_DB_WRITE 468
#define MOD_PARM_DEALLOC 469
#define INST_PARM_DEALLOC 470
#define SYS_TASK_UNIFORM 471
#define SYS_TASK_RTL_DIST_UNIFORM 472
#define SYS_TASK_SRANDOM 473
#define SYS_TASK_RANDOM 474
#define SYS_TASK_URANDOM 475
#define SYS_TASK_URANDOM_RANGE 476
#define SYS_TASK_REALTOBITS 477
#define SYS_TASK_BITSTOREAL 478
#define SYS_TASK_SHORTREALTOBITS 479
#define SYS_TASK_BITSTOSHORTREAL 480
#define SYS_TASK_ITOR 481
#define SYS_TASK_RTOI 482
#define SYS_TASK_STORE_PLUSARGS 483
#define SYS_TASK_TEST_PLUSARG 484
#define SYS_TASK_VALUE_PLUSARGS 485
#define SYS_TASK_DEALLOC 486
#define DEF_LOOKUP 487
#define IS_DEFINED 488
#define DEF_MATCH 489
#define DEF_START 490
#define DEFINE_MACRO 491
#define DO_DEFINE 492
#define DEF_IS_DONE 493
#define DEF_FINISH 494
#define DEF_UNDEFINE 495
#define INCLUDE_FILENAME 496
#define DO_INCLUDE 497
#define YYWRAP 498
#define RESET_PPLEXER 499
#define REENTRANT_COUNT_AFU_BITS 500
#define REENTRANT_STORE_DATA_BITS 501
#define REENTRANT_RESTORE_DATA_BITS 502
#define REENTRANT_CREATE 503
#define REENTRANT_DEALLOC 504
#define PARSE_READLINE 505
#define PARSE_DESIGN 506
#define PARSE_AND_SCORE_DUMPFILE 507
#define LEXER_KEYWORD_SYS_2001_CODE 508
#define OBFUSCATE_SET_MODE 509
#define OBFUSCATE_NAME 510
#define OBFUSCATE_DEALLOC 511
#define INFO_SET_VECTOR_ELEM_SIZE 512
#define INFO_DB_WRITE 513
#define INFO_DB_READ 514
#define ARGS_DB_READ 515
#define MESSAGE_DB_READ 516
#define MERGED_CDD_DB_READ 517
#define INFO_DEALLOC 518
#define ASSERTION_PARSE 519
#define ASSERTION_PARSE_ATTR 520
#define ASSERTION_GET_STATS 521
#define ASSERTION_DISPLAY_INSTANCE_SUMMARY 522
#define ASSERTION_INSTANCE_SUMMARY 523
#define ASSERTION_DISPLAY_FUNIT_SUMMARY 524
#define ASSERTION_FUNIT_SUMMARY 525
#define ASSERTION_DISPLAY_VERBOSE 526
#define ASSERTION_INSTANCE_VERBOSE 527
#define ASSERTION_FUNIT_VERBOSE 528
#define ASSERTION_REPORT 529
#define ASSERTION_GET_FUNIT_SUMMARY 530
#define ASSERTION_COLLECT 531
#define ASSERTION_GET_COVERAGE 532
#define LXT2_RD_EXPAND_INTEGER_TO_BITS 533
#define LXT2_RD_EXPAND_BITS_TO_INTEGER 534
#define LXT2_RD_ITER_RADIX 535
#define LXT2_RD_ITER_RADIX0 536
#define LXT2_RD_BUILD_RADIX 537
#define LXT2_RD_REGENERATE_PROCESS_MASK 538
#define LXT2_RD_PROCESS_BLOCK 539
#define LXT2_RD_INIT 540
#define LXT2_RD_CLOSE 541
#define LXT2_RD_GET_FACNAME 542
#define LXT2_RD_ITER_BLOCKS 543
#define LXT2_RD_LIMIT_TIME_RANGE 544
#define LXT2_RD_UNLIMIT_TIME_RANGE 545
#define TREE_ADD 546
#define TREE_FIND 547
#define TREE_REMOVE 548
#define TREE_DEALLOC 549
#define EXPRESSION_CREATE_TMP_VECS 550
#define EXPRESSION_CREATE_VALUE 551
#define EXPRESSION_CREATE 552
#define EXPRESSION_SET_VALUE 553
#define EXPRESSION_SET_SIGNED 554
#define EXPRESSION_RESIZE 555
#define EXPRESSION_GET_ID 556
#define EXPRESSION_GET_FIRST_LINE_EXPR 557
#define EXPRESSION_GET_LAST_LINE_EXPR 558
#define EXPRESSION_GET_CURR_DIMENSION 559
#define EXPRESSION_FIND_RHS_SIGS 560
#define EXPRESSION_FIND_PARAMS 561
#define EXPRESSION_FIND_ULINE_ID 562
#define EXPRESSION_FIND_EXPR 563
#define EXPRESSION_CONTAINS_EXPR_CALLING_STMT 564
#define EXPRESSION_GET_ROOT_STATEMENT 565
#define EXPRESSION_ASSIGN_EXPR_IDS 566
#define EXPRESSION_DB_WRITE 567
#define EXPRESSION_DB_WRITE_TREE 568
#define EXPRESSION_DB_READ 569
#define EXPRESSION_DB_MERGE 570
#define EXPRESSION_MERGE 571
#define EXPRESSION_STRING_OP 572
#define EXPRESSION_STRING 573
#define EXPRESSION_OP_FUNC__XOR 574
#define EXPRESSION_OP_FUNC__XOR_A 575
#define EXPRESSION_OP_FUNC__MULTIPLY 576
#define EXPRESSION_OP_FUNC__MULTIPLY_A 577
#define EXPRESSION_OP_FUNC__DIVIDE 578
#define EXPRESSION_OP_FUNC__DIVIDE_A 579
#define EXPRESSION_OP_FUNC__MOD 580
#define EXPRESSION_OP_FUNC__MOD_A 581
#define EXPRESSION_OP_FUNC__ADD 582
#define EXPRESSION_OP_FUNC__ADD_A 583
#define EXPRESSION_OP_FUNC__SUBTRACT 584
#define EXPRESSION_OP_FUNC__SUB_A 585
#define EXPRESSION_OP_FUNC__AND 586
#define EXPRESSION_OP_FUNC__AND_A 587
#define EXPRESSION_OP_FUNC__OR 588
#define EXPRESSION_OP_FUNC__OR_A 589
#define EXPRESSION_OP_FUNC__NAND 590
#define EXPRESSION_OP_FUNC__NOR 591
#define EXPRESSION_OP_FUNC__NXOR 592
#define EXPRESSION_OP_FUNC__LT 593
#define EXPRESSION_OP_FUNC__GT 594
#define EXPRESSION_OP_FUNC__LSHIFT 595
#define EXPRESSION_OP_FUNC__LSHIFT_A 596
#define EXPRESSION_OP_FUNC__RSHIFT 597
#define EXPRESSION_OP_FUNC__RSHIFT_A 598
#define EXPRESSION_OP_FUNC__ARSHIFT 599
#define EXPRESSION_OP_FUNC__ARSHIFT_A 600
#define EXPRESSION_OP_FUNC__TIME 601
#define EXPRESSION_OP_FUNC__RANDOM 602
#define EXPRESSION_OP_FUNC__SASSIGN 603
#define EXPRESSION_OP_FUNC__SRANDOM 604
#define EXPRESSION_OP_FUNC__URANDOM 605
#define EXPRESSION_OP_FUNC__URANDOM_RANGE 606
#define EXPRESSION_OP_FUNC__REALTOBITS 607
#define EXPRESSION_OP_FUNC__BITSTOREAL 608
#define EXPRESSION_OP_FUNC__SHORTREALTOBITS 609
#define EXPRESSION_OP_FUNC__BITSTOSHORTREAL 610
#define EXPRESSION_OP_FUNC__ITOR 611
#define EXPRESSION_OP_FUNC__RTOI 612
#define EXPRESSION_OP_FUNC__TEST_PLUSARGS 613
#define EXPRESSION_OP_FUNC__VALUE_PLUSARGS 614
#define EXPRESSION_OP_FUNC__EQ 615
#define EXPRESSION_OP_FUNC__CEQ 616
#define EXPRESSION_OP_FUNC__LE 617
#define EXPRESSION_OP_FUNC__GE 618
#define EXPRESSION_OP_FUNC__NE 619
#define EXPRESSION_OP_FUNC__CNE 620
#define EXPRESSION_OP_FUNC__LOR 621
#define EXPRESSION_OP_FUNC__LAND 622
#define EXPRESSION_OP_FUNC__COND 623
#define EXPRESSION_OP_FUNC__COND_SEL 624
#define EXPRESSION_OP_FUNC__UINV 625
#define EXPRESSION_OP_FUNC__UAND 626
#define EXPRESSION_OP_FUNC__UNOT 627
#define EXPRESSION_OP_FUNC__UOR 628
#define EXPRESSION_OP_FUNC__UXOR 629
#define EXPRESSION_OP_FUNC__UNAND 630
#define EXPRESSION_OP_FUNC__UNOR 631
#define EXPRESSION_OP_FUNC__UNXOR 632
#define EXPRESSION_OP_FUNC__NULL 633
#define EXPRESSION_OP_FUNC__SIG 634
#define EXPRESSION_OP_FUNC__SBIT 635
#define EXPRESSION_OP_FUNC__MBIT 636
#define EXPRESSION_OP_FUNC__EXPAND 637
#define EXPRESSION_OP_FUNC__LIST 638
#define EXPRESSION_OP_FUNC__CONCAT 639
#define EXPRESSION_OP_FUNC__PEDGE 640
#define EXPRESSION_OP_FUNC__NEDGE 641
#define EXPRESSION_OP_FUNC__AEDGE 642
#define EXPRESSION_OP_FUNC__EOR 643
#define EXPRESSION_OP_FUNC__SLIST 644
#define EXPRESSION_OP_FUNC__DELAY 645
#define EXPRESSION_OP_FUNC__TRIGGER 646
#define EXPRESSION_OP_FUNC__CASE 647
#define EXPRESSION_OP_FUNC__CASEX 648
#define EXPRESSION_OP_FUNC__CASEZ 649
#define EXPRESSION_OP_FUNC__DEFAULT 650
#define EXPRESSION_OP_FUNC__BASSIGN 651
#define EXPRESSION_OP_FUNC__FUNC_CALL 652
#define EXPRESSION_OP_FUNC__TASK_CALL 653
#define EXPRESSION_OP_FUNC__NB_CALL 654
#define EXPRESSION_OP_FUNC__FORK 655
#define EXPRESSION_OP_FUNC__JOIN 656
#define EXPRESSION_OP_FUNC__DISABLE 657
#define EXPRESSION_OP_FUNC__REPEAT 658
#define EXPRESSION_OP_FUNC__EXPONENT 659
#define EXPRESSION_OP_FUNC__PASSIGN 660
#define EXPRESSION_OP_FUNC__MBIT_POS 661
#define EXPRESSION_OP_FUNC__MBIT_NEG 662
#define EXPRESSION_OP_FUNC__NEGATE 663
#define EXPRESSION_OP_FUNC__IINC 664
#define EXPRESSION_OP_FUNC__PINC 665
#define EXPRESSION_OP_FUNC__IDEC 666
#define EXPRESSION_OP_FUNC__PDEC 667
#define EXPRESSION_OP_FUNC__DLY_ASSIGN 668
#define EXPRESSION_OP_FUNC__DLY_OP 669
#define EXPRESSION_OP_FUNC__REPEAT_DLY 670
#define EXPRESSION_OP_FUNC__DIM 671
#define EXPRESSION_OP_FUNC__WAIT 672
#define EXPRESSION_OP_FUNC__FINISH 673
#define EXPRESSION_OP_FUNC__STOP 674
#define EXPRESSION_OPERATE 675
#define EXPRESSION_OPERATE_RECURSIVELY 676
#define EXPRESSION_IS_STATIC_ONLY_HELPER 677
#define EXPRESSION_IS_ASSIGNED 678
#define EXPRESSION_IS_BIT_SELECT 679
#define EXPRESSION_IS_LAST_SELECT 680
#define EXPRESSION_IS_IN_RASSIGN 681
#define EXPRESSION_SET_ASSIGNED 682
#define EXPRESSION_SET_CHANGED 683
#define EXPRESSION_ASSIGN 684
#define EXPRESSION_DEALLOC 685
#define GEN_ITEM_STRINGIFY 686
#define GEN_ITEM_DISPLAY 687
#define GEN_ITEM_DISPLAY_BLOCK_HELPER 688
#define GEN_ITEM_DISPLAY_BLOCK 689
#define GEN_ITEM_COMPARE 690
#define GEN_ITEM_FIND 691
#define GEN_ITEM_REMOVE_IF_CONTAINS_EXPR_CALLING_STMT 692
#define GEN_ITEM_GET_GENVAR 693
#define GEN_ITEM_VARNAME_CONTAINS_GENVAR 694
#define GEN_ITEM_CALC_SIGNAL_NAME 695
#define GEN_ITEM_CREATE_EXPR 696
#define GEN_ITEM_CREATE_SIG 697
#define GEN_ITEM_CREATE_STMT 698
#define GEN_ITEM_CREATE_INST 699
#define GEN_ITEM_CREATE_TFN 700
#define GEN_ITEM_CREATE_BIND 701
#define GEN_ITEM_RESIZE_STMTS_AND_SIGS 702
#define GEN_ITEM_ASSIGN_IDS 703
#define GEN_ITEM_DB_WRITE 704
#define GEN_ITEM_DB_WRITE_EXPR_TREE 705
#define GEN_ITEM_CONNECT 706
#define GEN_ITEM_RESOLVE 707
#define GEN_ITEM_BIND 708
#define GENERATE_RESOLVE 709
#define GENERATE_REMOVE_STMT_HELPER 710
#define GENERATE_REMOVE_STMT 711
#define GEN_ITEM_DEALLOC 712
#define FSM_VAR_ADD 713
#define FSM_VAR_IS_OUTPUT_STATE 714
#define FSM_VAR_BIND_EXPR 715
#define FSM_VAR_ADD_EXPR 716
#define FSM_VAR_BIND_STMT 717
#define FSM_VAR_BIND_ADD 718
#define FSM_VAR_STMT_ADD 719
#define FSM_VAR_BIND 720
#define FSM_VAR_DEALLOC 721
#define FSM_VAR_REMOVE 722
#define FSM_VAR_CLEANUP 723
#define INSTANCE_DISPLAY_TREE_HELPER 724
#define INSTANCE_DISPLAY_TREE 725
#define INSTANCE_CREATE 726
#define INSTANCE_GEN_SCOPE 727
#define INSTANCE_COMPARE 728
#define INSTANCE_FIND_SCOPE 729
#define INSTANCE_FIND_BY_FUNIT 730
#define INSTANCE_FIND_SIGNAL_BY_EXCLUSION_ID 731
#define INSTANCE_FIND_EXPRESSION_BY_EXCLUSION_ID 732
#define INSTANCE_FIND_FSM_ARC_INDEX_BY_EXCLUSION_ID 733
#define INSTANCE_ADD_CHILD 734
#define INSTANCE_COPY 735
#define INSTANCE_ATTACH_CHILD 736
#define INSTANCE_PARSE_ADD 737
#define INSTANCE_RESOLVE_INST 738
#define INSTANCE_RESOLVE_HELPER 739
#define INSTANCE_RESOLVE 740
#define INSTANCE_READ_ADD 741
#define INSTANCE_DB_WRITE 742
#define INSTANCE_FLATTEN_HELPER 743
#define INSTANCE_FLATTEN 744
#define INSTANCE_REMOVE_STMT_BLKS_CALLING_STMT 745
#define INSTANCE_REMOVE_PARMS_WITH_EXPR 746
#define INSTANCE_DEALLOC_SINGLE 747
#define INSTANCE_OUTPUT_DUMPVARS 748
#define INSTANCE_DEALLOC_TREE 749
#define INSTANCE_DEALLOC 750
#define VECTOR_INIT 751
#define VECTOR_INT_R64 752
#define VECTOR_INT_R32 753
#define VECTOR_CREATE 754
#define VECTOR_COPY 755
#define VECTOR_COPY_RANGE 756
#define VECTOR_CLONE 757
#define VECTOR_DB_WRITE 758
#define VECTOR_DB_READ 759
#define VECTOR_DB_MERGE 760
#define VECTOR_MERGE 761
#define VECTOR_GET_EVAL_A 762
#define VECTOR_GET_EVAL_B 763
#define VECTOR_GET_EVAL_C 764
#define VECTOR_GET_EVAL_D 765
#define VECTOR_GET_EVAL_AB_COUNT 766
#define VECTOR_GET_EVAL_ABC_COUNT 767
#define VECTOR_GET_EVAL_ABCD_COUNT 768
#define VECTOR_GET_TOGGLE01_ULONG 769
#define VECTOR_GET_TOGGLE10_ULONG 770
#define VECTOR_DISPLAY_TOGGLE01_ULONG 771
#define VECTOR_DISPLAY_TOGGLE10_ULONG 772
#define VECTOR_TOGGLE_COUNT 773
#define VECTOR_MEM_RW_COUNT 774
#define VECTOR_SET_ASSIGNED 775
#define VECTOR_SET_COVERAGE_AND_ASSIGN 776
#define VECTOR_GET_SIGN_EXTEND_VECTOR_ULONG 777
#define VECTOR_SIGN_EXTEND_ULONG 778
#define VECTOR_LSHIFT_ULONG 779
#define VECTOR_RSHIFT_ULONG 780
#define VECTOR_SET_VALUE 781
#define VECTOR_PART_SELECT_PULL 782
#define VECTOR_PART_SELECT_PUSH 783
#define VECTOR_SET_UNARY_EVALS 784
#define VECTOR_SET_AND_COMB_EVALS 785
#define VECTOR_SET_OR_COMB_EVALS 786
#define VECTOR_SET_OTHER_COMB_EVALS 787
#define VECTOR_IS_UKNOWN 788
#define VECTOR_IS_NOT_ZERO 789
#define VECTOR_SET_TO_X 790
#define VECTOR_TO_INT 791
#define VECTOR_TO_UINT64 792
#define VECTOR_TO_REAL64 793
#define VECTOR_TO_SIM_TIME 794
#define VECTOR_FROM_INT 795
#define VECTOR_FROM_UINT64 796
#define VECTOR_FROM_REAL64 797
#define VECTOR_SET_STATIC 798
#define VECTOR_TO_STRING 799
#define VECTOR_FROM_STRING_FIXED 800
#define VECTOR_FROM_STRING 801
#define VECTOR_VCD_ASSIGN 802
#define VECTOR_BITWISE_AND_OP 803
#define VECTOR_BITWISE_NAND_OP 804
#define VECTOR_BITWISE_OR_OP 805
#define VECTOR_BITWISE_NOR_OP 806
#define VECTOR_BITWISE_XOR_OP 807
#define VECTOR_BITWISE_NXOR_OP 808
#define VECTOR_OP_LT 809
#define VECTOR_OP_LE 810
#define VECTOR_OP_GT 811
#define VECTOR_OP_GE 812
#define VECTOR_OP_EQ 813
#define VECTOR_CEQ_ULONG 814
#define VECTOR_OP_CEQ 815
#define VECTOR_OP_CXEQ 816
#define VECTOR_OP_CZEQ 817
#define VECTOR_OP_NE 818
#define VECTOR_OP_CNE 819
#define VECTOR_OP_LOR 820
#define VECTOR_OP_LAND 821
#define VECTOR_OP_LSHIFT 822
#define VECTOR_OP_RSHIFT 823
#define VECTOR_OP_ARSHIFT 824
#define VECTOR_OP_ADD 825
#define VECTOR_OP_NEGATE 826
#define VECTOR_OP_SUBTRACT 827
#define VECTOR_OP_MULTIPLY 828
#define VECTOR_OP_DIVIDE 829
#define VECTOR_OP_MODULUS 830
#define VECTOR_OP_INC 831
#define VECTOR_OP_DEC 832
#define VECTOR_UNARY_INV 833
#define VECTOR_UNARY_AND 834
#define VECTOR_UNARY_NAND 835
#define VECTOR_UNARY_OR 836
#define VECTOR_UNARY_NOR 837
#define VECTOR_UNARY_XOR 838
#define VECTOR_UNARY_NXOR 839
#define VECTOR_UNARY_NOT 840
#define VECTOR_OP_EXPAND 841
#define VECTOR_OP_LIST 842
#define VECTOR_DEALLOC_VALUE 843
#define VECTOR_DEALLOC 844
#define SYMTABLE_ADD_SYM_SIG 845
#define SYMTABLE_INIT 846
#define SYMTABLE_CREATE 847
#define SYMTABLE_ADD 848
#define SYMTABLE_SET_VALUE 849
#define SYMTABLE_ASSIGN 850
#define SYMTABLE_DEALLOC 851
#define FUNIT_INIT 852
#define FUNIT_CREATE 853
#define FUNIT_GET_CURR_MODULE 854
#define FUNIT_GET_CURR_MODULE_SAFE 855
#define FUNIT_GET_CURR_FUNCTION 856
#define FUNIT_GET_CURR_TASK 857
#define FUNIT_GET_PORT_COUNT 858
#define FUNIT_FIND_PARAM 859
#define FUNIT_FIND_SIGNAL 860
#define FUNIT_REMOVE_STMT_BLKS_CALLING_STMT 861
#define FUNIT_GEN_TASK_FUNCTION_NAMEDBLOCK_NAME 862
#define FUNIT_SIZE_ELEMENTS 863
#define FUNIT_DB_WRITE 864
#define FUNIT_DB_READ 865
#define FUNIT_VERSION_DB_READ 866
#define FUNIT_DB_MERGE 867
#define FUNIT_MERGE 868
#define FUNIT_FLATTEN_NAME 869
#define FUNIT_FIND_BY_ID 870
#define FUNIT_IS_TOP_MODULE 871
#define FUNIT_IS_UNNAMED 872
#define FUNIT_IS_UNNAMED_CHILD_OF 873
#define FUNIT_IS_CHILD_OF 874
#define FUNIT_DISPLAY_SIGNALS 875
#define FUNIT_DISPLAY_EXPRESSIONS 876
#define STATEMENT_ADD_THREAD 877
#define FUNIT_PUSH_THREADS 878
#define STATEMENT_DELETE_THREAD 879
#define FUNIT_OUTPUT_DUMPVARS 880
#define FUNIT_CLEAN 881
#define FUNIT_DEALLOC 882
#define ARC_FIND_FROM_STATE 883
#define ARC_FIND_TO_STATE 884
#define ARC_FIND_ARC 885
#define ARC_FIND_ARC_BY_EXCLUSION_ID 886
#define ARC_CREATE 887
#define ARC_ADD 888
#define ARC_STATE_HITS 889
#define ARC_TRANSITION_HITS 890
#define ARC_TRANSITION_EXCLUDED 891
#define ARC_GET_STATS 892
#define ARC_DB_WRITE 893
#define ARC_DB_READ 894
#define ARC_DB_MERGE 895
#define ARC_MERGE 896
#define ARC_GET_STATES 897
#define ARC_GET_TRANSITIONS 898
#define ARC_ARE_ANY_EXCLUDED 899
#define ARC_DEALLOC 900
#define LEXER_KEYWORD_SYS_1995_CODE 901
#define FUNC_ITER_DISPLAY 902
#define FUNC_ITER_SORT 903
#define FUNC_ITER_COUNT_STMT_ITERS 904
#define FUNC_ITER_ADD_STMT_ITERS 905
#define FUNC_ITER_ADD_SIG_LINKS 906
#define FUNC_ITER_INIT 907
#define FUNC_ITER_GET_NEXT_STATEMENT 908
#define FUNC_ITER_GET_NEXT_SIGNAL 909
#define FUNC_ITER_DEALLOC 910
#define EXCLUDE_EXPR_ASSIGN_AND_RECALC 911
#define EXCLUDE_SIG_ASSIGN_AND_RECALC 912
#define EXCLUDE_ARC_ASSIGN_AND_RECALC 913
#define EXCLUDE_ADD_EXCLUDE_REASON 914
#define EXCLUDE_REMOVE_EXCLUDE_REASON 915
#define EXCLUDE_IS_LINE_EXCLUDED 916
#define EXCLUDE_SET_LINE_EXCLUDE 917
#define EXCLUDE_IS_TOGGLE_EXCLUDED 918
#define EXCLUDE_SET_TOGGLE_EXCLUDE 919
#define EXCLUDE_IS_COMB_EXCLUDED 920
#define EXCLUDE_SET_COMB_EXCLUDE 921
#define EXCLUDE_IS_FSM_EXCLUDED 922
#define EXCLUDE_SET_FSM_EXCLUDE 923
#define EXCLUDE_IS_ASSERT_EXCLUDED 924
#define EXCLUDE_SET_ASSERT_EXCLUDE 925
#define EXCLUDE_FIND_EXCLUDE_REASON 926
#define EXCLUDE_DB_WRITE 927
#define EXCLUDE_DB_READ 928
#define EXCLUDE_RESOLVE_REASON 929
#define EXCLUDE_DB_MERGE 930
#define EXCLUDE_FIND_SIGNAL 931
#define EXCLUDE_FIND_EXPRESSION 932
#define EXCLUDE_FIND_FSM_ARC 933
#define EXCLUDE_FORMAT_REASON 934
#define EXCLUDED_GET_MESSAGE 935
#define EXCLUDE_HANDLE_EXCLUDE_REASON 936
#define EXCLUDE_PRINT_EXCLUSION 937
#define EXCLUDE_LINE_FROM_ID 938
#define EXCLUDE_TOGGLE_FROM_ID 939
#define EXCLUDE_MEMORY_FROM_ID 940
#define EXCLUDE_EXPR_FROM_ID 941
#define EXCLUDE_FSM_FROM_ID 942
#define EXCLUDE_ASSERT_FROM_ID 943
#define EXCLUDE_APPLY_EXCLUSIONS 944
#define COMMAND_EXCLUDE 945
#define RACE_BLK_CREATE 946
#define RACE_FIND_HEAD_STATEMENT_CONTAINING_STATEMENT_HELPER 947
#define RACE_FIND_HEAD_STATEMENT_CONTAINING_STATEMENT 948
#define RACE_GET_HEAD_STATEMENT 949
#define RACE_FIND_HEAD_STATEMENT 950
#define RACE_CALC_STMT_BLK_TYPE 951
#define RACE_CALC_EXPR_ASSIGNMENT 952
#define RACE_CALC_ASSIGNMENTS 953
#define RACE_HANDLE_RACE_CONDITION 954
#define RACE_CHECK_ASSIGNMENT_TYPES 955
#define RACE_CHECK_ONE_BLOCK_ASSIGNMENT 956
#define RACE_CHECK_RACE_COUNT 957
#define RACE_CHECK_MODULES 958
#define RACE_DB_WRITE 959
#define RACE_DB_READ 960
#define RACE_GET_STATS 961
#define RACE_REPORT_SUMMARY 962
#define RACE_REPORT_VERBOSE 963
#define RACE_REPORT 964
#define RACE_COLLECT_LINES 965
#define RACE_BLK_DELETE_LIST 966
#define STATISTIC_CREATE 967
#define STATISTIC_IS_EMPTY 968
#define STATISTIC_DEALLOC 969
#define TCL_FUNC_GET_RACE_REASON_MSGS 970
#define TCL_FUNC_GET_FUNIT_LIST 971
#define TCL_FUNC_GET_INSTANCES 972
#define TCL_FUNC_GET_INSTANCE_LIST 973
#define TCL_FUNC_IS_FUNIT 974
#define TCL_FUNC_GET_FUNIT 975
#define TCL_FUNC_GET_INST 976
#define TCL_FUNC_GET_FUNIT_NAME 977
#define TCL_FUNC_GET_FILENAME 978
#define TCL_FUNC_INST_SCOPE 979
#define TCL_FUNC_GET_FUNIT_START_AND_END 980
#define TCL_FUNC_COLLECT_UNCOVERED_LINES 981
#define TCL_FUNC_COLLECT_COVERED_LINES 982
#define TCL_FUNC_COLLECT_RACE_LINES 983
#define TCL_FUNC_COLLECT_UNCOVERED_TOGGLES 984
#define TCL_FUNC_COLLECT_COVERED_TOGGLES 985
#define TCL_FUNC_COLLECT_UNCOVERED_MEMORIES 986
#define TCL_FUNC_COLLECT_COVERED_MEMORIES 987
#define TCL_FUNC_GET_TOGGLE_COVERAGE 988
#define TCL_FUNC_GET_MEMORY_COVERAGE 989
#define TCL_FUNC_COLLECT_UNCOVERED_COMBS 990
#define TCL_FUNC_COLLECT_COVERED_COMBS 991
#define TCL_FUNC_GET_COMB_EXPRESSION 992
#define TCL_FUNC_GET_COMB_COVERAGE 993
#define TCL_FUNC_COLLECT_UNCOVERED_FSMS 994
#define TCL_FUNC_COLLECT_COVERED_FSMS 995
#define TCL_FUNC_GET_FSM_COVERAGE 996
#define TCL_FUNC_COLLECT_UNCOVERED_ASSERTIONS 997
#define TCL_FUNC_COLLECT_COVERED_ASSERTIONS 998
#define TCL_FUNC_GET_ASSERT_COVERAGE 999
#define TCL_FUNC_OPEN_CDD 1000
#define TCL_FUNC_CLOSE_CDD 1001
#define TCL_FUNC_SAVE_CDD 1002
#define TCL_FUNC_MERGE_CDD 1003
#define TCL_FUNC_GET_LINE_SUMMARY 1004
#define TCL_FUNC_GET_TOGGLE_SUMMARY 1005
#define TCL_FUNC_GET_MEMORY_SUMMARY 1006
#define TCL_FUNC_GET_COMB_SUMMARY 1007
#define TCL_FUNC_GET_FSM_SUMMARY 1008
#define TCL_FUNC_GET_ASSERT_SUMMARY 1009
#define TCL_FUNC_PREPROCESS_VERILOG 1010
#define TCL_FUNC_GET_SCORE_PATH 1011
#define TCL_FUNC_GET_INCLUDE_PATHNAME 1012
#define TCL_FUNC_GET_GENERATION 1013
#define TCL_FUNC_SET_LINE_EXCLUDE 1014
#define TCL_FUNC_SET_TOGGLE_EXCLUDE 1015
#define TCL_FUNC_SET_MEMORY_EXCLUDE 1016
#define TCL_FUNC_SET_COMB_EXCLUDE 1017
#define TCL_FUNC_FSM_EXCLUDE 1018
#define TCL_FUNC_SET_ASSERT_EXCLUDE 1019
#define TCL_FUNC_GENERATE_REPORT 1020
#define TCL_FUNC_INITIALIZE 1021
#define STATIC_EXPR_GEN_UNARY 1022
#define STATIC_EXPR_GEN 1023
#define STATIC_EXPR_CALC_LSB_AND_WIDTH_PRE 1024
#define STATIC_EXPR_CALC_LSB_AND_WIDTH_POST 1025
#define STATIC_EXPR_DEALLOC 1026
#define VCDID 1027
#define VCD_CALLBACK 1028
#define LXT_PARSE 1029
#define COMMAND_MERGE 1030
#define LINE_GET_STATS 1031
#define LINE_COLLECT 1032
#define LINE_GET_FUNIT_SUMMARY 1033
#define LINE_GET_INST_SUMMARY 1034
#define LINE_DISPLAY_INSTANCE_SUMMARY 1035
#define LINE_INSTANCE_SUMMARY 1036
#define LINE_DISPLAY_FUNIT_SUMMARY 1037
#define LINE_FUNIT_SUMMARY 1038
#define LINE_DISPLAY_VERBOSE 1039
#define LINE_INSTANCE_VERBOSE 1040
#define LINE_FUNIT_VERBOSE 1041
#define LINE_REPORT 1042

extern profiler profiles[NUM_PROFILES];
#endif

extern unsigned int profile_index;

#endif

