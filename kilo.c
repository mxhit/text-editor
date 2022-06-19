/*** INCLUDES ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** DEFINES ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** DATA ***/

struct termios orig_termios;

/*** TERMINAL ***/

// Error handling
void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s); // printing the string 's' along with the error description based on the value of errno 
	exit(1); // exiting the program with the status 1, indicating a failure
}

void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}

// Disabling a bunch of flags to enable raw mode 
void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = orig_termios;
	raw.c_lflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_lflag &= ~(OPOST);
	raw.c_lflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey() {
	int nread;
	char c;

	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}

	return c;
}

/*** OUTPUT ***/

void editorDrawRows() {
	int y;
	for (y = 0; y < 24; y++) {
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}

// drawing "~" for 24 rows
void editorRefreshScreen() {
	// writing out 4 bytes to the terminal
	// first byte is "\x1b" which is the Escape character <ESC> (27 in decimal), the other three bytes are "[2J"
	// any escape sequence starts with the escape character followed by a "["
	// the "J" command is used to erase the screen
	// Escape sequence commands take arguments which come before the command, which in our case is "2" which says clear the entire screen
	write(STDOUT_FILENO, "\x1b[2J", 4);

	// repositioning the cursor to the top left corner
	// the "H" command is used to position the cursor. It take 2 arguments: row number, column number. we are not passing any arguments so by default it will consider it as 1st row, 1st column
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();

	write(STDOUT_FILENO, "\x1b[H", 3); // repositioning the cursor after tildes have been drawn
}

/*** INPUT ***/

void editorProcessKeypress() {
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);

			exit(0);
			break;
	}
}

/*** INIT ***/

int main() {
	enableRawMode();

	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}