#include "speedrun.h"
#include "console.h"
#include "doomdef.h"
#include "g_game.h"
#include "r_things.h"
#include <stdio.h>
#include <stdbool.h>
#include "credentials.h"

// Exits with an error
void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  //exit(1);
}

// Converts the time from tics into a string mm:ss.cc
char *time_to_string(int time)
{
    char *time_string = malloc(TIME_STRING_LEN);
    snprintf(time_string, TIME_STRING_LEN,
        "%i:%02i.%02i"
        , G_TicsToMinutes(time,true)
        , G_TicsToSeconds(time)
        , G_TicsToCentiseconds(time));
    return time_string;
}

// Insert score into the database
void insert_score(MYSQL *con, int mapnum, char* username, char* skin, int time)
{
    // setup the statement handler and statement binds
    MYSQL_STMT* stmt = mysql_stmt_init(con);
    MYSQL_BIND bind[5];

    if (mysql_stmt_prepare(stmt, INSERT_SCORE, strlen(INSERT_SCORE))) {
        finish_with_error(con);
    }

    // reset the binds
    memset(bind, 0, sizeof(bind));
    unsigned long username_length = strlen(username);
    unsigned long skin_length = strlen(skin);
    char * time_string = time_to_string(time);
    unsigned long  time_string_length = strlen(time_string);

    // bind the new time(int)
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&time;
    bind[0].is_null = 0;
    bind[0].length = 0;

    // bind the new time(string mm:ss.cc)
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = time_string;
    bind[1].buffer_length = time_string_length;
    bind[1].is_null = 0;
    bind[1].length = &time_string_length;

    // bind the player's username
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = username;
    bind[2].buffer_length = username_length;
    bind[2].is_null = 0;
    bind[2].length = &username_length;

    // bind the player's current character
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = skin;
    bind[3].buffer_length = skin_length;
    bind[3].is_null = 0;
    bind[3].length = &skin_length;

    // bind the map's number
    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = (char *)&mapnum;
    bind[4].is_null = 0;
    bind[4].length = 0;

    // execute the statement
    if (mysql_stmt_bind_param(stmt, bind)) {
        finish_with_error(con);
    }
    if (mysql_stmt_execute(stmt)) {
        finish_with_error(con);
    }

    // close the statement
    if (mysql_stmt_close(stmt)) {
        finish_with_error(con);
    }

    // deallocate the memory for the time(string mm:ss.cc)
    free(time_string);

    //debug, comment if unnecessary
    //printf("Inserted %s's time to the database\n", username);

}

// Insert the map if it is not yet in the database
/*
void insert_map(MYSQL *con, int mapnum, char *mapname)
{
    char query[QUERY_LEN];

    // setup the results of the tables, the statement handler and the statement bind
    MYSQL_RES *result;
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[2];

    // get the map's row in the table
    snprintf(query, QUERY_LEN, GET_MAP, mapnum);
    if (mysql_query(con, query)) {
        finish_with_error(con);
    }

    // get the result of the sql tables
    result = mysql_store_result(con);
    // if no result was found
    if (result == NULL)
    {
        finish_with_error(con);
    }

    // if the map doesn't exist in the database
    if (mysql_num_rows(result) == 0) {
        // create the inserter
        stmt = mysql_stmt_init(con);

        // insert the map
        if (mysql_stmt_prepare(stmt, INSERT_MAP, strlen(INSERT_MAP))) {
            finish_with_error(con);
        }

        // reset the binds
        memset(bind, 0, sizeof(bind));
        unsigned long str_length = strlen(mapname);

        // add the mapnum to the binds
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = (char *)&mapnum;
        bind[0].is_null = 0;
        bind[0].length = 0;

        // add the map's name to the binds
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = mapname;
        bind[1].buffer_length = strlen(mapname);
        bind[1].is_null = 0;
        bind[1].length = &str_length;

        // set the binds to the statement handler and executes the statement
        if (mysql_stmt_bind_param(stmt, bind)) {
            finish_with_error(con);
        }
        if (mysql_stmt_execute(stmt)) {
            finish_with_error(con);
        }

        // closes the statement handler
        if (mysql_stmt_close(stmt)) {
            finish_with_error(con);
        }
    }

    // free the memory allocated for the tables result
    mysql_free_result(result);

    //debug, comment if unnecessary
    //printf("Map by ID %d and name %s added to \"maps\" table\n", mapnum, mapname);
}
*/

// Process the time of the player
void process_time(MYSQL *con, int playernum, int mapnum)
{
    // get the player's time, username, character
    int time = players[playernum].realtime;
    char* username = player_names[playernum];
    char* skin = ((skin_t *)players[playernum].mo->skin)->name;

    // insert the current time
    insert_score(con, mapnum, username, skin, time);
}

// Called when the race has finished
void speedrun_map_completed()
{
    if (gametype != GT_RACE) return; //only RACE gametype has "numlaps" value

    // create the mysql handler
    MYSQL *con = mysql_init(NULL);

    // get the map's number
    int mapnum = gamemap - 1;
    char mapname[30];

    // check for the number of the act and prints the level's name
    if (mapheaderinfo[mapnum]->actnum) {
        snprintf(mapname, 30, "%s %d", mapheaderinfo[mapnum]->lvlttl, mapheaderinfo[mapnum]->actnum);
    } else {
        snprintf(mapname, 30, "%s", mapheaderinfo[mapnum]->lvlttl);
    }

    if (con == NULL)
    {
        // print the mysql error
        finish_with_error(con);
        exit(1);
    }

    // connect to the database
    if (mysql_real_connect(con, "127.0.0.1", USERNAME, PASSWORD, DATABASE, 0, NULL, 0) == NULL)
    {
        finish_with_error(con);
    }

    // inserts the new map if not found
    //insert_map(con, mapnum, mapname);

    //for every player
    for (unsigned char playernum = 0; playernum < MAXPLAYERS; playernum++) {
        // if the player is not in game, is a spectator or is dead: skip it
    	if ((!playeringame[playernum]) || (players[playernum].spectator) || (players[playernum].pflags & PF_GAMETYPEOVER) || (players[playernum].laps < (unsigned)cv_numlaps.value)) //ruins calculation for all non-circuit maps
    	    continue;

    	// else save its time
    	process_time(con, playernum, mapnum);
    }

    // closes the connection
    mysql_close(con);
}

/*
void init_string(struct string *s)
{
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t write_to_string(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}
*/

msg_buf_t msg_buf = {
    .index = 0
};

void add_message(char *msg)
{
    if (msg_buf.index >= MSG_BUF_LEN) {
        fprintf(stderr, "Error: message buffer full\n");
        return;
    }
    msg_buf.msgs[msg_buf.index++] = msg;
}

void send_message()
{
    if (msg_buf.index != 0) {
        char *msg = msg_buf.msgs[--msg_buf.index];
        SendNetXCmd(XD_SAY, msg, strlen(msg + 2) + 3);
        free(msg);
    }
}

//executes with "MapLoad" lua hook
void send_best_time()
{
    printf("Gamemap: %d\n", gamemap);

    MYSQL *con;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[256];

    con = mysql_init(NULL);

    if (con == NULL)
    {
        // print the mysql error
        finish_with_error(con);
        exit(1);
    }

    // connect to the database
    if (!mysql_real_connect(con, "127.0.0.1", USERNAME, PASSWORD, DATABASE, 0, NULL, 0))
    {
        finish_with_error(con);
    }

    //send query
    snprintf(query, 256, "SELECT username, skin, MIN(time_string) FROM highscores WHERE map_id = %d GROUP BY skin", gamemap - 1);
    if (mysql_query(con, query))
    {
       finish_with_error(con);
    }

    //recieve result
    res = mysql_use_result(con);

    while (row = mysql_fetch_row(res))
    {
	//row[0] is username
	//row[1] is skin
	//row[2] is time

        char *buf = malloc(MSG_LEN);
        char *msg = &buf[2];
        const size_t msgspace = MSG_LEN - 2;

        buf[0] = 0; // send message to everyone
        buf[1] = 1; // send message as server

        // sending the data via "Server C To Client Lua" channel
        snprintf(msg, msgspace, "SCTCL%s,%s,%s", row[0], row[1], row[2]);
        add_message(buf);
    }
}
