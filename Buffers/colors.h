#ifndef _COLORS_H
#define _COLORS_H
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_BLACK "\x1b[30m"

#define ANSI_BLUE_BACKGROUND "\x1b[44m"
#define ANSI_RED_BACKGROUND "\x1b[41m"
#define ANSI_GREEN_BACKGROUND "\x1b[42m"
#define ANSI_PINK_BACKGROUND "\x1b[101m"
#define ANSI_CYAN_BACKGROUND "\x1b[46m"
#define ANSI_GRAY_BACKGROUND "\x1b[100m"
#define ANSI_LIGHT_GREEN_BACKGROUND "\x1b[106m"
#define ANSI_MAGENTA_GREEN_BACKGROUND "\x1b[45m"

#endif
// printf("%c[%d;%dmHello World%c[%dm\n",27,1,33,27,0);