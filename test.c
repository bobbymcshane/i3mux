#include <stdio.h>
#include <string.h>
#include <glib/gprintf.h>
#include <json-glib/json-glib.h>
#include <i3ipc-glib/i3ipc-glib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "tmux_commands.h"
#include "parse.h"
#include <string.h>

typedef unsigned int uint_t;
#define I3_WORKSPACE_ADD_CMD "workspace %d, exec gnome-terminal"

gint main() {
     i3ipcConnection *conn;
     gchar *reply = NULL;

     conn = i3ipc_connection_new(NULL, NULL);
     char buf[BUFSIZ];
     char tmux_cmd[BUFSIZ];
     char i3_cmd[BUFSIZ];
     char bufchar;
     char endc;
     int workspace, n_scanned, pane;

     pid_t i;
     int fds, fdm, status;

     /* TRY OPENING A SINGLE TTY AND PUMPING THE OUTPUT TO IT */
#if 0

     /* Open a new unused tty */
     fdm = posix_openpt(O_RDWR);
     grantpt(fdm);
     unlockpt(fdm);

     printf("pts/%s\n", ptsname(fdm));
     //close(0); /* Don't close stdin */
     close(1);
     close(2);

     i = fork();
     if ( i == 0 ) { // parent
          dup(fdm);
          dup(fdm);
          dup(fdm);
          //waitpid(i, &status, 0);
     } else {  // child
          fds = open(ptsname(fdm), O_RDWR);
          dup(fds);
          dup(fds);
          dup(fds);
          strcpy(buf, (ptsname(fdm)));
          /* Spawn a urxvt terminal which looks at the specified pty */
          sprintf(buf, "urxvt -pty-fd %c/2", basename(buf));
          system(buf);
          exit(0);
     }
     /* END PRINT TO OTHER TERMINAL TEST */
#endif
     while ( 1 ) {
          if (scanf( "%%%s ", tmux_cmd ) == 1 ) {
               /* REACHED LINE END */
               if ( !strcmp( tmux_cmd, TMUX_WINDOW_ADD ) ) {
                    if (scanf( "@%d", &workspace ) == 1 ) {
                         sprintf( i3_cmd, I3_WORKSPACE_ADD_CMD, workspace );
                         reply = i3ipc_connection_message(conn, I3IPC_MESSAGE_TYPE_COMMAND, i3_cmd, NULL);
                         //g_printf("Reply: %s\n", reply);
                         g_free(reply);
                    }
               }
               else if ( !strcmp( tmux_cmd, "output" ) ) {
                    if (scanf( "%%%d", &pane ) == 1) {
                         fgetc(stdin); /* READ A SINGLE SPACE FROM AFTER THE PANE */
                         fgets( buf, BUFSIZ, stdin); 
                         //printf( "%s", buf);
                         buf[(strlen(buf)-1)] = '\0';
                         printf( "%s", unescape(buf));
                         fflush(stdout);
                    }
               }
               else if ( !strcmp( tmux_cmd, TMUX_LAYOUT_CHANGE ) ) {
                    if (scanf( "@%d", &workspace ) == 1 ) {
                         /* TODO: Do I need to remove the newline at the end of the layout string? */
                         /* retrieve the entire layout string */
                         fgets( buf, BUFSIZ, stdin);
                         char* parse_str = buf;
                         /* Ignore checksum for now */
                         char checksum[5];
                         sscanf( parse_str, "%4s,", checksum );
                         parse_str += 6;
                         
                         gchar *layout_str = tmux_layout_to_i3_layout( parse_str );

                         char tmpfile[] = "/tmp/layout_XXXXXX";
                         int layout_fd = mkstemp( tmpfile );
                         fchmod( layout_fd, 0666 );
                         dprintf( layout_fd, "%s", layout_str );
                         close( layout_fd );
                         g_free( layout_str );
                         sprintf( i3_cmd, "workspace %d, append_layout %s", workspace, tmpfile );
                         reply = i3ipc_connection_message(conn, I3IPC_MESSAGE_TYPE_COMMAND, i3_cmd, NULL);
                         //g_printf("Reply: %s\n", reply);
                         g_free(reply);
                         /* Strategy move window to mark then kill the marked pane */
                         //remove ( tmpfile );
                         //sprintf( i3_cmd, "exec gnome-terminal" );
#if 0
                         reply = i3ipc_connection_message(conn, I3IPC_MESSAGE_TYPE_COMMAND, i3_cmd, NULL);
                         g_printf("Reply: %s\n", reply);
                         g_free(reply);
#endif
                    }
               }
          }
          else {
next_cmd:
               /* Seek to end of line */
               fgets( buf, BUFSIZ, stdin); 
               //printf("%s", buf);
          }
          bufchar = fgetc( stdin );
          if ( bufchar == EOF )
               return 0;
          else
               ungetc( bufchar, stdin );
     }

     g_object_unref(conn);

     return 0;
}

