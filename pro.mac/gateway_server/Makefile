INCLUDE = -I./../hiredis/include -I./../mysql-connector-c/include
gateway_server: ../../gataway_server/main.c \
	../../gataway_server/from_client.c  \
        ../../gataway_server/gw_config.c \
        ../../gataway_server/server_post.c \
        ../../gataway_server/server_return.c \
        ../../gataway_server/session_key_man.c \
        ../../gataway_server/server_session.c \
        ../../3rd/crypt/base64_decode.c \
	../../3rd/crypt/base64_encoder.c  \
        ../../3rd/crypt/sha1.c \
        ../../3rd/md5/md5.c \
        ../../3rd/mjson/json.c \
        ../../3rd/mjson/json_extends.c \
        ../../3rd/http_parser/http_parser.c \
	../../utils/cache_allocer.c \
	../../utils/hash_int_map.c \
	../../utils/log.c \
	../../utils/ramdom.c \
	../../utils/timestamp.c \
	../../utils/vector_array.c \
	../../utils/timer_list.c \
        \
	../../netbus/netbus.c \
	../../netbus/socket/session.c \
	../../netbus/socket/server_uv.c \
	\
	../../game_command.c \
	#../../database/center_db_config.c \
	#../../database/center_database.c \
	#../../database/game_db_config.c \
	#../../database/game_database.c 

	gcc $(INCLUDE) -std=c99 -g -Wall -o $@ $^ -lpthread -luv -D_GNU_SOURCE -L./../libuv/lib/ -L./../hiredis/lib -lhiredis -L./../mysql-connector-c/lib -lmysqlclient

clean:
	rm gateway_server
