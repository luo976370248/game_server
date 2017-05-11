#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

static uv_loop_t* loop;
static uv_tcp_t tcpServer;
static uv_handle_t* server;
static uv_connect_t* connect_req;

static char ip_address[64];
static int ip_port;

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "libuv.lib")
#endif

#define my_malloc malloc
#define my_free free
#define my_realloc realloc

#define MAX_PKG_SIZE ((1<<16) - 1)
#define MAX_RECV_SIZE 2048


#include "session.h"
#include "../../3rd/http_parser/http_parser.h"
#include "../../3rd/crypt/sha1.h"
#include "../../3rd/crypt/base64_encoder.h"
#include "../../utils/timer_list.h"

#include "../netbus.h"
#include "../../utils/log.h"

extern struct timer_list* GATEWAY_TIMER_LIST;


char *wb_accept = "HTTP/1.1 101 Switching Protocols\r\n" \
"Upgrade:websocket\r\n" \
"Connection: Upgrade\r\n" \
"Sec-WebSocket-Accept: %s\r\n" \
"WebSocket-Location: ws://%s:%d/chat\r\n" \
"WebSocket-Protocol:chat\r\n\r\n";

extern void
on_json_protocal_recv_entry(struct session* s, unsigned char* data, int len);

extern void
on_bin_protocal_recv_entry(struct session* s, unsigned char* data, int len);


extern void
init_server_gateway();

extern void
exit_server_gateway();

void
close_session(struct session* s);

struct io_package {
	struct session* s;
	int recved; // �յ����ֽ���;
	unsigned char* long_pkg;
	int max_pkg_len;
};

typedef struct {
	uv_write_t req;
	uv_buf_t buf;
} write_req_t;

static void
after_write(uv_write_t* req, int status) {
	write_req_t* wr;

	/* Free the read/write buffer and the request */
	wr = (write_req_t*)req;
	my_free(wr->buf.base);
	my_free(wr);

	if (status == 0)
		return;

	fprintf(stderr,
		"uv_write error: %s - %s\n",
		uv_err_name(status),
		uv_strerror(status));
}

void
socket_send_data(void* ud, unsigned char* buf, int nread) {
	uv_stream_t* handle = (uv_stream_t*)ud;
	write_req_t *wr;
	wr = (write_req_t*)my_malloc(sizeof(write_req_t));

	int alloc_size = (nread < 2048) ? 2048 : nread;
	unsigned char* send_buf = my_malloc(alloc_size);
	memcpy(send_buf, buf, nread);
	wr->buf = uv_buf_init(send_buf, nread);

	if (uv_write(&wr->req, handle, &wr->buf, 1, after_write)) {
		LOGERROR("uv_write failed");
	}
}

static void
uv_close_stream(uv_handle_t* peer) {
	struct io_package* io_data = peer->data;
	if (io_data->s != NULL) {
		io_data->s->c_sock = NULL;
		remove_session(io_data->s);
		io_data->s = NULL;
	}

	if (peer->data != NULL) {
		my_free(peer->data);
		peer->data = NULL;
	}

	my_free(peer);
}

// ������ͷ�Ļص�����
static char header_key[64];
static char client_ws_key[128];
static int has_client_key = 0;
static int
on_header_field(http_parser* p, const char *at, size_t length) {
	length = (length < 63) ? length : 63;
	strncpy(header_key, at, length);
	header_key[length] = 0;
	// printf("%s:", header_key);
	return 0;
}

static int
on_header_value(http_parser* p, const char *at,
	size_t length) {
	if (strcmp(header_key, "Sec-WebSocket-Key") != 0) {
		return 0;
	}
	length = (length < 127) ? length : 127;

	strncpy(client_ws_key, at, length);
	client_ws_key[length] = 0;
	// printf("%s\n", client_ws_key);
	has_client_key = 1;

	return 0;
}


static int
process_ws_shake_hand(struct session* ses, struct io_package* io_data,
	char* ip, int port) {
	http_parser p;
	http_parser_init(&p, HTTP_REQUEST);

	http_parser_settings s;
	http_parser_settings_init(&s);
	s.on_header_field = on_header_field;
	s.on_header_value = on_header_value;

	has_client_key = 0;
	unsigned char* data = io_data->long_pkg;
	http_parser_execute(&p, &s, data, io_data->recved);

	// ���û���õ�key_migic,��ʾ����û�����꣬����ȥ�����ֵ�����;
	if (has_client_key == 0) {
		ses->is_shake_hand = 0;
		if (io_data->recved >= MAX_RECV_SIZE) { // �����������ְ�;
			close_session(ses->c_sock);
			// uv_close(ses->c_sock, uv_close_stream);
			return -1;
		}
		return -1;
	}

	// ��һ��http�����ݸ����ǵ�client,����websocket����
	static char key_migic[256];
	const char* migic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	sprintf(key_migic, "%s%s", client_ws_key, migic);

	int sha1_size = 0; // ��ż��ܺ�����ݳ���
	int base64_len = 0;
	uint8_t sha1_content[SHA1_DIGEST_SIZE];
	crypt_sha1(key_migic, (int)strlen(key_migic), sha1_content, &sha1_size);
	char* b64_str = base64_encode(sha1_content, sha1_size, &base64_len);
	// end 
	strncpy(key_migic, b64_str, base64_len);
	key_migic[base64_len] = 0;
	// printf("key_migic = %s\n", key_migic);

	// �����http�ı��Ļظ����ǵ�websocket��������Ŀͻ��ˣ�
	// ����websocket���ӡ�
	static char accept_buffer[256];
	sprintf(accept_buffer, wb_accept, key_migic, ip, port);
	socket_send_data(ses->c_sock, accept_buffer, (int)strlen(accept_buffer));
	ses->is_shake_hand = 1;
	base64_encode_free(b64_str);

	if (io_data->long_pkg != NULL) {
		my_free(io_data->long_pkg);
		io_data->long_pkg = NULL;

	}
	io_data->recved = 0;
	io_data->max_pkg_len = 0;

	return 0;
}

static int
recv_ws_header(unsigned char* pkg_data, int pkg_len,
	int* pkg_size, int* out_header_size) {
	// ��һ���ֽ���ͷ���Ѿ��жϣ�����;
	// end 

	unsigned char* mask = NULL;
	unsigned char* raw_data = NULL;

	if (pkg_len < 2) { // websocket ��ͷû������
		return -1;
	}

	unsigned int len = pkg_data[1];

	// ��ߵ�һ��bitʼ��Ϊ1,����Ҫ����ߵ����bit,��Ϊ0;
	len = (len & 0x0000007f);
	if (len <= 125) { // 4��mask�ֽڣ�ͷ������������
		if (pkg_len < (2 + 4)) { // �޷������� ��С��mask��ֵ
			return -1;
		}
		mask = pkg_data + 2; // ͷ�ֽڣ������ֽ�
	}
	else if (len == 126) { // ���������ֽڱ�ʾ���ȣ�
		if (pkg_len < (4 + 4)) { // 1 + 1 + 2�������ֽ� + 4 MASK
			return -1;
		}
		// len = ((pkg_data[2]) | (pkg_data[3] << 8));
		len = ((pkg_data[3]) | (pkg_data[2] << 8));
		mask = pkg_data + 2 + 2;
	}
	// 1 + 1 + 8(������Ϣ) + 4MASK
	else if (len == 127) { // ����������ÿ���,����ǰ4���ֽڵĴ�С�����治��;
		if (pkg_len < 14) {
			return -1;
		}
		unsigned int low = ((pkg_data[5]) | (pkg_data[4] << 8) | (pkg_data[3] << 16) | (pkg_data[2] << 24));
		unsigned int hight = ((pkg_data[9]) | (pkg_data[8] << 8) | (pkg_data[7] << 16) | (pkg_data[6] << 24));
		if (hight != 0) { // ��ʾ���ĸ��ֽ�������int��Ų��ˣ�̫���ˣ����ǲ�Ҫ�ˡ�
			return -1;
		}
		len = low;
		mask = pkg_data + 2 + 8;
	}
	// mask �̶�4���ֽڣ����Ժ�������ݲ���
	raw_data = mask + 4;
	*out_header_size = (int)(raw_data - pkg_data);
	*pkg_size = len + (*out_header_size);
	// printf("data length = %d\n", len);
	return 0;
}

static void
parser_ws_pack(struct session* s,
	unsigned char* body, int body_len,
	unsigned char* mask, int protocal_type) {
	// ʹ��mask,�����ݻ�ԭ������
	for (int i = 0; i < body_len; i++) {
		body[i] = body[i] ^ mask[i % 4]; // maskֻ��4���ֽڵĳ��ȣ����ԣ�Ҫѭ��ʹ�ã����������ȡ��Ϳ����ˡ�
	}
	// end

	// ������ȥ���������ڷֺ��ˡ�
	if (protocal_type == JSON_PROTOCAL) {
		on_json_protocal_recv_entry(s, body, body_len);
	}
	else if (protocal_type == BIN_PROTOCAL) {
		on_bin_protocal_recv_entry(s, body, body_len);
	}
	// end
}

static void
on_ws_pack_recved(struct session* s, struct io_package* io_data, int protocal_type) {
	// Step1: �������ݵ�ͷ����ȡ������Ϸ��Э�����Ĵ�С;
	while (io_data->recved > 0) {
		int pkg_size = 0;
		int header_size = 0;
		unsigned char* pkg_data = io_data->long_pkg;

		if (recv_ws_header(pkg_data, io_data->recved, &pkg_size, &header_size) != 0) { // ����Ͷ��recv����֪���ܷ����һ������ͷ;
			break;
		}

		// Step2:�ж����ݴ�С���Ƿ񲻷��Ϲ涨�ĸ�ʽ
		if (pkg_size >= MAX_PKG_SIZE) { // ,�쳣�����ݰ���ֱ�ӹرյ�socket;
										// uv_close(s->c_sock, uv_close_stream);
			close_session(s);
			break;
		}

		// �Ƿ�������һ�����ݰ�;
		if (io_data->recved >= pkg_size) { // ��ʾ�����Ѿ��յ����ٳ�����һ���������ݣ�
										   // 0x81
			if (pkg_data[0] == 0x88) { // �Է�Ҫ�ر�socket
									   // uv_close(s->c_sock, uv_close_stream);
				close_session(s);
				break;
			}

			parser_ws_pack(s, pkg_data + header_size,
				pkg_size - header_size, pkg_data + header_size - 4, protocal_type);

			if (io_data->recved > pkg_size) { // 1.5 ����
				memmove(io_data->long_pkg, io_data->long_pkg + pkg_size, io_data->recved - pkg_size);
			}
			io_data->recved -= pkg_size;

			if (io_data->recved == 0 && io_data->long_pkg != NULL) {
				my_free(io_data->long_pkg);
				io_data->long_pkg = NULL;
				io_data->max_pkg_len = 0;
			}
			break;
		}
	}
}

static int
read_pkg_tail(unsigned char* pkg_data, int recv, int* pkg_size) {
	if (recv < 2) { // �����ܴ��\r\n
		return -1;
	}

	int i = 0;
	*pkg_size = 0;

	while (i < recv - 1) {
		if (pkg_data[i] == '\r' && pkg_data[i + 1] == '\n') {
			*pkg_size = (i + 2);
			return 0;
		}
		i++;
	}

	return -1;
}

static int
recv_header(unsigned char* pkg, int len, int* pkg_size) {
	if (len <= 1) { // �յ������ݲ��ܹ������ǵİ��Ĵ�С��������
		return -1;
	}

	*pkg_size = (pkg[0]) | (pkg[1] << 8);
	return 0;
}

static void
on_bin_protocal_recved(struct session* s, struct io_package* io_data) {
	// Step1: �������ݵ�ͷ����ȡ������Ϸ��Э�����Ĵ�С;
	while (io_data->recved > 0) {
		int pkg_size = 0;
		if (recv_header(io_data->long_pkg, io_data->recved, &pkg_size) != 0) { // ����Ͷ��recv����֪���ܷ����һ������ͷ;
			break;
		}

		// Step2:�ж����ݴ�С���Ƿ񲻷��Ϲ涨�ĸ�ʽ
		if (pkg_size >= MAX_PKG_SIZE) { // ,�쳣�����ݰ���ֱ�ӹرյ�socket;
										// uv_close(s->c_sock, uv_close_stream);
			close_session(s->c_sock);
			break;
		}

		// �Ƿ�������һ�����ݰ�;
		if (io_data->recved >= pkg_size) { // ��ʾ�����Ѿ��յ����ٳ�����һ���������ݣ�
			unsigned char* pkg_data = io_data->long_pkg;

			// on_server_recv(s, pkg_data + 2, pkg_size - 2);
			on_bin_protocal_recv_entry(s, pkg_data + 2, pkg_size - 2);

			if (io_data->recved > pkg_size) { // 1.5 ����
				memmove(io_data->long_pkg, io_data->long_pkg + pkg_size, io_data->recved - pkg_size);
			}
			io_data->recved -= pkg_size;

			if (io_data->recved == 0 && io_data->long_pkg != NULL) {
				my_free(io_data->long_pkg);
				io_data->long_pkg = NULL;
				io_data->max_pkg_len = 0;
			}
		}
		// end 
	}
}

static void
on_json_protocal_recved(struct session* s, struct io_package* io_data) {
	while (io_data->recved > 0) {
		int pkg_size = 0;
		unsigned char* pkg_data = io_data->long_pkg;

		if (read_pkg_tail(pkg_data, io_data->recved, &pkg_size) != 0) { // û�ж���\r\n
			if (io_data->recved >= (((1 << 16) - 1))) { // ���������ݰ�,close session
														// uv_close(s->c_sock, uv_close_stream);
				close_session(s->c_sock);
			}
			break;
		}
		// �������������,�ҵ�������������, io_data->pkg,��ʼ,  pkg_size
		// end 
		// on_server_recv_line(s, pkg_data, pkg_size);
		on_json_protocal_recv_entry(s, pkg_data, pkg_size);

		if (io_data->recved > pkg_size) {
			memmove(io_data->long_pkg, io_data->long_pkg + pkg_size, io_data->recved - pkg_size);
		}
		io_data->recved -= pkg_size;
		if (io_data->recved == 0 && io_data->long_pkg != NULL) {
			my_free(io_data->long_pkg);
			io_data->long_pkg = NULL;
			io_data->max_pkg_len = 0;
		}
	} // end while
}

static void
uv_buf_alloc(uv_handle_t* handle,
	size_t suggested_size,
	uv_buf_t* buf) {
	struct io_package* io_data = (struct io_package*)handle->data;
	int alloc_len = (io_data->recved + suggested_size);
	alloc_len = (alloc_len > ((1 << 16) - 1)) ? ((1 << 16) - 1) : alloc_len;
	if (alloc_len < MAX_RECV_SIZE) {
		alloc_len = MAX_RECV_SIZE;
	}

	if ((alloc_len) > io_data->max_pkg_len) { // pkg,�Ų�����,
		io_data->long_pkg = my_realloc(io_data->long_pkg, alloc_len + 1);
		io_data->max_pkg_len = alloc_len;
	}
	unsigned char* data = io_data->long_pkg;
	buf->base = data + io_data->recved;
	buf->len = suggested_size;
}

static void
after_shutdown(uv_shutdown_t* req, int status) {
	uv_close((uv_handle_t*)req->handle, uv_close_stream);
	my_free(req);
}

static void
my_add_timer(int msec);
static uv_timer_t timer_req;
static void timer_callback(uv_timer_t *handle) {
	int m_sec = update_timer_list(GATEWAY_TIMER_LIST);
	if (m_sec > 0) {
		my_add_timer(m_sec);
	}
}

static void
my_add_timer(int msec) {
	uv_timer_start(&timer_req, timer_callback, msec, 1);
}

static void after_read(uv_stream_t* handle,
	ssize_t nread,
	const uv_buf_t* buf) {

	uv_shutdown_t* sreq;

	if (nread < 0) {
		/* Error or EOF */
		// assert(nread == UV_EOF);
		sreq = my_malloc(sizeof(uv_shutdown_t));
		uv_shutdown(sreq, handle, after_shutdown);
		return;
	}

	if (nread == 0) {
		/* Everything OK, but nothing read. */
		// my_free(buf->base);
		return;
	}

	struct io_package* io_data = handle->data;
	if (io_data) {
		io_data->recved += nread;
	}

	int protocal_type = get_proto_type();
	struct session* s = io_data->s;

	if (s->socket_type == TCP_SOCKET_IO) {
		if (protocal_type == BIN_PROTOCAL) {
			on_bin_protocal_recved(s, io_data);
		}
		else if (protocal_type == JSON_PROTOCAL) {
			on_json_protocal_recved(s, io_data);
		}
	}
	else if (s->socket_type == WEB_SOCKET_IO) { // ����web socket,�Լ���ʡ�Ͷ����˷�������ĸ�ʽ;
		if (s->is_shake_hand == 0) { // websocket��û�����ֳɹ�,��������;
			process_ws_shake_hand(s, io_data, ip_address, ip_port);
		}
		else { // ����websocket���͹��������ݰ���
			on_ws_pack_recved(s, io_data, protocal_type);
		}
	}

	// cancel timer adjust timer
	int m_sec = update_timer_list(GATEWAY_TIMER_LIST);
	if (m_sec > 0) {
		my_add_timer(m_sec);
	}
	// end 
}

static void on_connection(uv_stream_t* server, int status) {
	uv_stream_t* stream;
	int r;

	if (status != 0) {
		LOGERROR("Connect error %s\n", uv_err_name(status));
		return;
	}

	stream = my_malloc(sizeof(uv_tcp_t));

	r = uv_tcp_init(loop, (uv_tcp_t*)stream);
	r = uv_accept(server, stream);
	r = uv_read_start(stream, uv_buf_alloc, after_read);

	struct io_package* io_data;
	io_data = my_malloc(sizeof(struct io_package));
	stream->data = io_data;
	io_data->max_pkg_len = MAX_RECV_SIZE;
	memset(stream->data, 0, sizeof(struct io_package));

	struct session* s = save_session(stream, "127.0.0.1", 100);
	io_data->s = s;
	io_data->long_pkg = NULL;

	s->socket_type = get_socket_type();
}

static int tcp4_server_start(uv_loop_t* loop, int port) {
	struct sockaddr_in addr;
	int r;

	uv_ip4_addr("0.0.0.0", port, &addr);

	server = (uv_handle_t*)&tcpServer;

	r = uv_tcp_init(loop, &tcpServer);
	if (r) {
		/* TODO: Error codes */
		LOGERROR("Socket creation error\n");
		return 1;
	}

	r = uv_tcp_bind(&tcpServer, (const struct sockaddr*) &addr, 0);
	if (r) {
		/* TODO: Error codes */
		LOGERROR("Bind error\n");
		return 1;
	}

	r = uv_listen((uv_stream_t*)&tcpServer, SOMAXCONN, on_connection);
	if (r) {
		/* TODO: Error codes */
		LOGERROR("Listen error %s\n", uv_err_name(r));
		return 1;
	}

	return 0;
}

void
start_server(char*ip, int port) {
	int socket_type, protocal_type;

	socket_type = get_socket_type();
	protocal_type = get_proto_type();
	strcpy(ip_address, ip);
	ip_port = port;

	loop = uv_default_loop();
	uv_timer_init(uv_default_loop(), &timer_req);
	int msec = update_timer_list(GATEWAY_TIMER_LIST);
	if (msec > 0) {
		my_add_timer(msec);
	}
	if (tcp4_server_start(loop, port)) {
		return;
	}

	uv_run(loop, UV_RUN_DEFAULT);
}

static void
after_connect(uv_connect_t* handle, int status) {
	if (status) {
		LOGERROR("connect error");
		uv_close((uv_handle_t*)handle->handle, uv_close_stream);
		return;
	}

	int iret = uv_read_start(handle->handle, uv_buf_alloc, after_read);
	if (iret) {
		LOGERROR("uv_read_start error");
		return;
	}
}

struct session*
	netbus_connect(char* server_ip, int port) {

	struct sockaddr_in bind_addr;
	int iret = uv_ip4_addr(server_ip, port, &bind_addr);
	if (iret) {
		return NULL;
	}

	uv_tcp_t* stream = my_malloc(sizeof(uv_tcp_t));
	uv_tcp_init(loop, stream);

	struct io_package* io_data;
	io_data = my_malloc(sizeof(struct io_package));
	memset(io_data, 0, sizeof(struct io_package));
	stream->data = io_data;

	struct session* s = save_session(stream, server_ip, port);
	io_data->max_pkg_len = 0;
	io_data->s = s;
	io_data->long_pkg = NULL;
	s->socket_type = TCP_SOCKET_IO;
	s->is_server_session = 1;

	connect_req = my_malloc(sizeof(uv_connect_t));
	iret = uv_tcp_connect(connect_req, stream, (struct sockaddr*)&bind_addr, after_connect);
	if (iret) {
		LOGERROR("uv_tcp_connect error!!!");
		return NULL;
	}

	return s;
}

void
close_session(struct session* s) {
	if (s->c_sock) {
		uv_shutdown_t* sreq = my_malloc(sizeof(uv_shutdown_t));
		uv_shutdown(sreq, s->c_sock, after_shutdown);
	}
}
