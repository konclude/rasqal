/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rasqal_internal.h - Rasqal RDF Query library internals
 *
 * $Id$
 *
 * Copyright (C) 2003 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.org/
 * University of Bristol - http://www.bristol.ac.uk/
 * 
 * This package is Free Software or Open Source available under the
 * following licenses (these are alternatives):
 *   1. GNU Lesser General Public License (LGPL)
 *   2. GNU General Public License (GPL)
 *   3. Mozilla Public License (MPL)
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * full license terms.
 * 
 */



#ifndef RASQAL_INTERNAL_H
#define RASQAL_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RASQAL_INTERNAL

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RASQAL_MEMORY_DEBUG_DMALLOC)
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#include <dmalloc.h>
#endif

/* FIXME for testing */
#include <redland.h>

#define RASQAL_MALLOC(type, size) malloc(size)
#define RASQAL_CALLOC(type, size, count) calloc(size, count)
#define RASQAL_FREE(type, ptr)   free((void*)ptr)

#ifdef RASQAL_DEBUG
/* Debugging messages */
#define RASQAL_DEBUG1(msg) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__); } while(0)
#define RASQAL_DEBUG2(msg, arg1) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1);} while(0)
#define RASQAL_DEBUG3(msg, arg1, arg2) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2);} while(0)
#define RASQAL_DEBUG4(msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2, arg3);} while(0)
#define RASQAL_DEBUG5(msg, arg1, arg2, arg3, arg4) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2, arg3, arg4);} while(0)
#define RASQAL_DEBUG6(msg, arg1, arg2, arg3, arg4, arg5) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2, arg3, arg4, arg5);} while(0)

#if defined(HAVE_DMALLOC_H) && defined(RASQAL_MEMORY_DEBUG_DMALLOC)
void* rasqal_system_malloc(size_t size);
void rasqal_system_free(void *ptr);
#define SYSTEM_MALLOC(size)   rasqal_system_malloc(size)
#define SYSTEM_FREE(ptr)   rasqal_system_free(ptr)
#else
#define SYSTEM_MALLOC(size)   malloc(size)
#define SYSTEM_FREE(ptr)   free(ptr)
#endif

#else
/* DEBUGGING TURNED OFF */

/* No debugging messages */
#define RASQAL_DEBUG1(msg)
#define RASQAL_DEBUG2(msg, arg1)
#define RASQAL_DEBUG3(msg, arg1, arg2)
#define RASQAL_DEBUG4(msg, arg1, arg2, arg3)
#define RASQAL_DEBUG5(msg, arg1, arg2, arg3, arg4)
#define RASQAL_DEBUG6(msg, arg1, arg2, arg3, arg4, arg5)

#define SYSTEM_MALLOC(size)   malloc(size)
#define SYSTEM_FREE(ptr)   free(ptr)

#endif


/* Fatal errors - always happen */
#define RASQAL_FATAL1(msg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , __func__); abort();} while(0)
#define RASQAL_FATAL2(msg,arg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , __func__, arg); abort();} while(0)


#ifdef RASQALDEBUG
#define YYDEBUG 1
#define YYERROR_VERBOSE
#endif


typedef struct rasqal_query_engine_factory_s rasqal_query_engine_factory;


struct rasqal_triples_match_s {
  void *user_data;
  librdf_statement* (*get_match)(struct rasqal_triples_match_s*, void *user_data);
  void (*next_match)(struct rasqal_triples_match_s*, void *user_data);
  int (*is_end)(struct rasqal_triples_match_s*, void *user_data);
  void (*finish)(struct rasqal_triples_match_s*, void *user_data);
};
typedef struct rasqal_triples_match_s rasqal_triples_match;


typedef struct 
{
  /* All the parts of this triple are nodes - no variables */
  int is_exact;
  librdf_node* nodes[3];
  /* query statement, made from the nodes above (even when exact) */
  librdf_statement *qstatement;
  librdf_stream *stream;
  rasqal_variable* bindings[3];

  rasqal_triples_match *triples_match;
} rasqal_triple_meta;


/*
 * A query in some query language
 */
struct rasqal_query_s {
  unsigned char *query_string;

  raptor_namespace_stack *namespaces;

  /* sequences of ... */
  rasqal_sequence *selects;     /* ... rasqal_variable* names only */
  rasqal_sequence *sources;     /* ... raptor_uri*                 */
  rasqal_sequence *triples;     /* ... rasqal_triple*              */
  rasqal_sequence *constraints; /* ... rasqal_expression*          */
  rasqal_sequence *prefixes;    /* ... rasqal_prefix*              */

  int select_all;  /* non-0 if 'SELECT *' was seen (selects will be NULL) */
  
  /* variable name/value table built from all distinct variables seen
   * in selects, triples, constraints.  An array of size variables_count
   *
   * The first select_variables_count of this array are from the selects
   * and are typically returned to the user.
   */
  rasqal_variable **variables;
  int variables_count;
  int select_variables_count;

  /* holds one copy of all the variables - this is where they are freed */
  rasqal_sequence* variables_sequence;


  /* A reordered list of conjunctive triples from triples above
   * used as a better order to join in.
   *
   * NOTE: Shares the rasqal_triple* pointers with 'triples'.
   * The entries in this sequence are not freed.
   */
  rasqal_sequence *ordered_triples;

  /* An array of items, one per triple in a query */
  rasqal_triple_meta *triple_meta;

  /* the expression version of the sequence of constraints above - this is
   * where the constraints are freed
   */
  rasqal_expression* constraints_expression;

  /* can be filled with error location information */
  raptor_locator locator;

  /* base URI of this query for resolving relative URIs in queries */
  raptor_uri* base_uri;

  /* non 0 if parser had fatal error and cannot continue */
  int failed;

  /* stuff for our user */
  void *user_data;

  void *fatal_error_user_data;
  void *error_user_data;
  void *warning_user_data;

  raptor_message_handler fatal_error_handler;
  raptor_message_handler error_handler;
  raptor_message_handler warning_handler;

  /* query engine specific stuff */
  void *context;

  /* stopping? */
  int abort;
  
  /* FIXME querying a Redland model - for testing */
  librdf_world *world;
  librdf_model *model;

  struct rasqal_query_engine_factory_s* factory;
};


/*
 * A query engine factory for a query language
 */
struct rasqal_query_engine_factory_s {
  struct rasqal_query_engine_factory_s* next;

  /* query language name */
  const char* name;

  /* query language readable label */
  const char* label;

  /* query language alternate name */
  const char* alias;

  /* query language MIME type (or NULL) */
  const char* mime_type;

  /* query language URI (or NULL) */
  const unsigned char* uri_string;
  
  /* the rest of this structure is populated by the
     query-engine-specific register function */
  size_t context_length;
  
  /* create a new query */
  int (*init)(rasqal_query* rq, const char *name);
  
  /* destroy a query */
  void (*terminate)(rasqal_query* rq);
  
  /* prepare a query */
  int (*prepare)(rasqal_query* rq);
  
  /* execute a query */
  int (*execute)(rasqal_query* rq);

  /* finish the query engine factory */
  void (*finish_factory)(rasqal_query_engine_factory* factory);
};


/* rasqal_general.c */
char* rasqal_vsnprintf(const char *message, va_list arguments);

void rasqal_query_engine_register_factory(const char *name, const char *label, const char *alias, const unsigned char *uri_string, void (*factory) (rasqal_query_engine_factory*));
rasqal_query_engine_factory* rasqal_get_query_engine_factory (const char *name, const unsigned char *uri);

void rasqal_query_fatal_error(rasqal_query* query, const char *message, ...);
void rasqal_query_fatal_error_varargs(rasqal_query* query, const char *message, va_list arguments);
void rasqal_query_error(rasqal_query* query, const char *message, ...);
void rasqal_query_simple_error(void* query, const char *message, ...);
void rasqal_query_error_varargs(rasqal_query* query, const char *message, va_list arguments);
void rasqal_query_warning(rasqal_query* query, const char *message, ...);
void rasqal_query_warning_varargs(rasqal_query* query, const char *message, va_list arguments);

/* rdql_parser.y */
void rasqal_init_query_engine_rdql (void);

/* rasqal_engine.c */
int rasqal_query_order_triples(rasqal_query* query);
int rasqal_engine_declare_prefixes(rasqal_query *rq);
int rasqal_engine_expand_triple_qnames(rasqal_query* rq);
int rasqal_engine_expand_constraints_qnames(rasqal_query* rq);
int rasqal_engine_build_constraints_expression(rasqal_query* rq);
int rasqal_engine_assign_variables(rasqal_query* rq);
int rasqal_engine_run(rasqal_query *q);

/* end of RASQAL_INTERNAL */
#endif


#ifdef __cplusplus
}
#endif

#endif
