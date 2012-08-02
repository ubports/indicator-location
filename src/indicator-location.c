
#include <libappindicator/app-indicator.h>


GMainLoop * mainloop = NULL;

int
main (int argc, char * argv[])
{
	g_type_init();

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_main_loop_unref(mainloop);
	mainloop = NULL;

	return 0;
}
