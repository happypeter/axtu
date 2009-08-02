#ifndef TRACE_H_
#define TRACE_H_

/******************************************************************************/
/* MACRO for message string                                                   */
/******************************************************************************/
/* TRACE */ 
#define MSG_TRC                   "TRACE"

/* ERROR */
#define MSG_ERR                   "ERROR"
#define MSG_ERR_KIND_PRG          "kind=PRG"
#define MSG_ERR_KIND_ENV          "kind=ENV"
#define MSG_ERR_KIND_USR          "kind=USR"

/* DEBUG */
#define MSG_DBG                   "DEBUG"

/******************************************************************************/
/* MACRO for printing message                                                 */
/******************************************************************************/
/* TRACE */

#define TRC_PRINT(fp, msg) \
	fprintf(fp, "%s %s %s:%s:%s:%d:%s\n", \
					__DATE__, __TIME__,	MSG_TRC, __FILE__, \
					__FUNCTION__, __LINE__, msg)

#define TRC_PRINT_FUNC_START(fp) \
	fprintf(fp, "%s %s %s:%s:%s:%d: %s >>>\n", \
					__DATE__, __TIME__, MSG_TRC, __FILE__, \
					__FUNCTION__, __LINE__, __FUNCTION__)

#define TRC_PRINT_FUNC_END(fp) \
	fprintf(fp, "%s %s %s:%s:%s:%d: %s <<<\n", \
					__DATE__, __TIME__,	MSG_TRC, __FILE__, \
					__FUNCTION__, __LINE__, __FUNCTION__)

/* ERROR */
#define ERR_PRINT(fp, msg) \
	fprintf(fp, "%s %s %s:%s:%s:%d:%s\n", \
					__DATE__, __TIME__,	MSG_ERR, __FILE__, \
					__FUNCTION__, __LINE__, msg)

#define ERR_PRINT_KIND_PRG(fp, msg, ret, errno) \
  fprintf(fp, "%s %s %s:%s:%s:%d:%s:%s:ret=%d:errno=%d\n", \
					__DATE__, __TIME__,	MSG_ERR, __FILE__, \
					__FUNCTION__, __LINE__, MSG_ERR_KIND_PRG, msg, ret, errno)

#define ERR_PRINT_KIND_ENV(fp, msg) \
	fprintf(fp, "%s %s %s:%s:%s:%d:%s:%s\n", \
					__DATE__, __TIME__,	MSG_ERR, __FILE__, \
					__FUNCTION__, __LINE__, MSG_ERR_KIND_ENV, msg)

#define ERR_PRINT_KIND_USR(fp, msg) \
  fprintf(fp, "%s %s %s:%s:%s:%d:%s:%s\n", \
					__DATE__, __TIME__,	MSG_ERR, __FILE__, \
					__FUNCTION__, __LINE__, MSG_ERR_KIND_USR, msg)

/* DEBUG */
#define DBG_PRINT(fp, msg) \
	fprintf(fp, "%s %s %s:%s:%s:%d:%s\n",	\
					__DATE__, __TIME__,	MSG_ERR, __FILE__, \
					__FUNCTION__, __LINE__, msg)

#endif /* TRACE_H_ */
