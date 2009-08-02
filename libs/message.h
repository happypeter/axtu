/*!
@file message.h
@brief message macro file
*/

#ifndef MESSAGE_H
#define MESSAGE_H

#define DELIMITER ','
#define COMMENT   '#'
#define SPACE     ' '
#define NEWLINE   '\n'
#define MAX_LEN 1024
#define MIN_ELEMENT 2
#define MAX_ELEMENT 32

#define ERR_ENV_INCMP_PARSE_OVER_ELEMENT  "Incompatible config file should have less than max elements."
#define ERR_ENV_INCMP_PARSE_UNDER_ELEMENT "Incompatible config file should have more than min elements."
#define ERR_ENV_INCMP_PARSE_DUPE_ELEMENT  "Incompatible config file has duplicate elements"
#define ERR_ENV_INCMP_PARSE_OVER_LENGTH   "Incompatible config file has the line over the length."

#endif
