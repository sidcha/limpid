#include <limpid.h>
#include <unistd.h>

int cmd_ping(int argc, char *argv[], string_t **resp)
{
	int i;
	string_t *s = new_string(128);

	string_printf(s, "a", "pong");
	for (i=0; i<argc; i++) {
		string_printf(s, "a", "\n[%d] %s", i, argv[i]);
	}

	*resp = s;
	return 0;
}

int cmd_devshell(int arc, char *argv[], string_t **resp)
{
	execl("/bin/bash", "-i");
	return 0;
}

int main(int argc, char *argv[])
{
	limpid_server_init("/tmp/limpid-server");

	limpid_create_command("ping", cmd_ping);
	limpid_create_command("devshell", cmd_devshell);

	while (1) {
		// Your application code!
	}
	return 0;
}

