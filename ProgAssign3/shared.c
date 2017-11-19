#define CLI_MAX 100
#define NAME_MAX 20
#define MSG_MAX 1000

int str_to_int(const char *s);
void get_name_from_serv_info(char *name, const char *serv_info);
void get_dir_list(char *list, char info[CLI_MAX][MSG_MAX], const int info_size);
void append_to_string(char *orig, const char *a); 
uint16_t get_port_from_serv_info(const char *serv_info); 

int str_to_int(const char *s)
{
	int res = 0;
	while (*s) {
		res *= 10;
		res += *s++ - '0';
	}
	return res;
}

void get_name_from_serv_info(char *name, const char *serv_info) {
	int index = -1;
	const char *ptr = strchr(serv_info, ';');
	if(ptr) {
		index = ptr - serv_info;
	}

	snprintf(name, index + 1, "%s", serv_info);
}

uint16_t get_port_from_serv_info(const char *serv_info) {
	char port[MSG_MAX];
	memset(port, 0, MSG_MAX);

	int index = -1;
	const char *ptr = strchr(serv_info, ';');
	if(ptr) {
		index = ptr - serv_info;
	}

	memcpy(port, &serv_info[index + 1], MSG_MAX);

	return str_to_int(port);
}


void get_dir_list(char *list, char info[CLI_MAX][MSG_MAX], const int info_size) {
	snprintf(list, 21, "Chat Server Directory");

	for(int i = 0; i < info_size; i++) {
		append_to_string(list, info[i]);
	}
}

void append_to_string(char *orig, const char *a) {
	char temp[MSG_MAX];
	memset(temp, 0, MSG_MAX);
	snprintf(temp, MSG_MAX, "%s\n%s", orig, a);

	memset(orig, 0, MSG_MAX);
	strcpy(orig, temp);
}
