#ifndef CONSOLEUTILS_H
#define CONSOLEUTILS_H

/* Console ANSI escape codes to give color to the text in the (Linux?) console.
 * https://en.wikipedia.org/wiki/ANSI_escape_code
 *
 * Can be used in printout with for example printf, cout or qDebug().
 * Note: don't forget to write CONS_RESET reset the format and prevent the rest of the text having the same format.
  */

#define CONS_BLACK_NORMAL "\x1b[30m"
#define CONS_RED_NORMAL "\x1b[31m"
#define CONS_GRAY_BOLD "\x1b[30;1m"
#define CONS_RED_BOLD "\x1b[31;1m"
#define CONS_GREEN_BOLD "\x1b[32;1m"
#define CONS_RESET "\x1b[0m"

#endif // CONSOLEUTILS_H

