#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "eggtrayicon.h"
//intl stuff
#include<libintl.h>
#include<locale.h>
#include<signal.h>


#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop(String)

#define UPDATE_ICON "/usr/share/axtu/gui/images/updateicon.png"
#define EXEC_UPDATER "/usr/bin/kdesu \"/usr/share/axtu/gui/axtu-gui -u\""
#define EXEC_SETUP "/usr/bin/kdesu /usr/share/axtu/gui/axtu-setup-gui"
#define SETUP_ICON "/usr/share/axtu/gui/images/Icon_setup_n.png"

#define	NOTIFIER_GUI_PID_FILE  "/var/tmp/axtu-notifier-gui.pid"
#define	NOTIFIER_GUI_EXE_FILE "axtu-notifier-gui"
#define NOTIFIER_UPDATE_COUNT_TMP  "/var/axtu/axtu-update-count"
#define CHECK_NOTIFIER_DAEMON  "kill -USR1 `/sbin/pidof axtu-daemon` 2> /dev/null"
#define AUTHENTICATION_FILE		"/var/axtu/asianux-auth"
#define AUTHENTICATION_CONFIG_FILE	"/var/axtu/axtu-authen-client.conf"

#define NOTIFIER_TITLE _("Notifier - Asianux TSN Updater")


#define MAX_STRING 256

EggTrayIcon * mainIcon;
GtkWidget * menu;
GtkTooltips * tips;
GtkWidget * evbox;
GtkWidget * animation;
GdkPixbufAnimation *panibuf;
gboolean hasMouse=FALSE;

int g_UpdateCount=0;

void showMenu(guint button, guint32 time) {
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, time);
}

gboolean buttonUp(GtkWidget * w, GdkEventButton * e, gpointer d) {
	if (hasMouse) {	
		// click left button or double click		
		if (e->button == 1 ) {			
			gboolean retval;
			retval = g_spawn_command_line_async(EXEC_UPDATER,NULL);
			
			if (retval == FALSE) {
				GtkWidget *dialog;
				dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK, _("Fail to execute Updater"));
				gtk_window_set_title(dialog, NOTIFIER_TITLE );
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				return TRUE;
			}
		}
		// click right button
		else if (e->button == 3) {
			showMenu(e->button, e->time);
		}
	}
	return TRUE;
}

gboolean enterNotify(GtkWidget * w, GdkEventCrossing * e, gpointer d) {
	hasMouse=TRUE;
	return TRUE;
}

gboolean leaveNotify(GtkWidget * w, GdkEventCrossing * e, gpointer d) {
	hasMouse=FALSE;
	return TRUE;
}

void startSetup(GtkWidget * w, gchar * command) {
	gboolean retval;
	retval = g_spawn_command_line_async(command, NULL);
	if (retval == FALSE) {
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK, _("Fail to execute Setup"));
		gtk_window_set_title(dialog, NOTIFIER_TITLE );
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}

GtkWidget * make_image(gchar * path) {
	GdkPixbuf * p;
	gint w, h;
	gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &w, &h);
	p = gdk_pixbuf_new_from_file_at_size(path, w, h, NULL);
	if (!p) {
		return gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE,
				GTK_ICON_SIZE_MENU);
	} else {
		return gtk_image_new_from_pixbuf(p);
	}
}

void quitMe(GtkWidget * w, gpointer p) {
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO , _("Do you want to exit?"));
	gtk_window_set_title(dialog, NOTIFIER_TITLE );
	if( gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES)
	{	
		gtk_widget_destroy(tips);
		gtk_widget_destroy(menu);		
		gtk_widget_destroy(panibuf);
		gtk_widget_destroy(animation);
		gtk_widget_destroy(evbox);		
		gtk_widget_destroy(mainIcon);		
		gtk_main_quit();	
	}
	gtk_widget_destroy(dialog);
	
}

void init_menu() {
	GtkWidget * mitem;
	menu = gtk_menu_new();
	g_object_ref(menu);

	mitem = gtk_image_menu_item_new_with_label(_("Asianux TSN Updater setting"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(mitem),
			make_image(SETUP_ICON));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
	gtk_widget_show(mitem);
	g_signal_connect(mitem, "activate", G_CALLBACK(startSetup), EXEC_SETUP);

	mitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	g_signal_connect(mitem, "activate", G_CALLBACK(quitMe), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
	gtk_widget_show(mitem);

}

void init_widgets(int nType) {
	tips = gtk_tooltips_new();
	mainIcon = egg_tray_icon_new(_("Asianux TSN Updater"));
	evbox = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(evbox), FALSE);
	char buf[MAX_STRING];

	if ((access(AUTHENTICATION_FILE, F_OK) != 0
			&& access(AUTHENTICATION_CONFIG_FILE, F_OK) == 0) || nType == -1) {
		snprintf(buf, sizeof(buf), "%s : \n%s\n%s\n", _("Asianux TSN Updater"), _("Update is not registered."), _("Please register now"));
	} else {
		snprintf(buf, sizeof(buf), "%s : \n%s : [%d]\n%s\n", _("Asianux TSN Updater"), _("Update available pakages"), g_UpdateCount, _("Please update now"));
	}

	gtk_tooltips_set_tip(tips, evbox, _(buf), _("Asianux TSN Updater"));
	panibuf = gdk_pixbuf_animation_new_from_file(UPDATE_ICON,NULL);

	animation = gtk_image_new_from_animation(panibuf);
	gtk_container_add(GTK_CONTAINER(mainIcon), evbox);
	gtk_container_add(GTK_CONTAINER(evbox), animation);
	g_signal_connect(evbox, "button-release-event", G_CALLBACK(buttonUp), NULL);
	g_signal_connect(evbox, "enter-notify-event", G_CALLBACK(enterNotify), NULL);
	g_signal_connect(evbox, "leave-notify-event", G_CALLBACK(leaveNotify), NULL);
	g_signal_connect(mainIcon, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);		
	init_menu();
}

void show_widgets(int nType) {
	if (!mainIcon) {
		init_widgets(nType);
	} else {
		char buf[MAX_STRING];
		if (nType == -1) {
			snprintf(buf, sizeof(buf), "%s : \n%s\n%s\n", _("Asianux TSN Updater"), _("Update is not registered."), _("Please register now"));
		} else {
			snprintf(buf, sizeof(buf), "%s : \n%s : [%d]\n%s\n", _("Asianux TSN Updater"), _("Update available pakages"), g_UpdateCount, _("Please update now"));
		}
		gtk_tooltips_set_tip(tips, evbox, _(buf), _("Asianux TSN Updater"));
	}
	gtk_widget_show_all(GTK_WIDGET(mainIcon));

}
void hide_widgets() {

	if (mainIcon) {
		gtk_widget_hide_all(GTK_WIDGET(mainIcon));
		gtk_widget_destroy(mainIcon);
		mainIcon=NULL;
	}
}

static void sig_ChangeStatus(int signo) {
	if (signo != SIGUSR1)
		return;

	char szReadBuf[MAX_STRING];
	int fd;

	if (access(AUTHENTICATION_FILE, F_OK) != 0) {
		show_widgets(-1);
		return;
	}

	// File is not exsist , That file is for update checking.
	if (access(NOTIFIER_UPDATE_COUNT_TMP, F_OK) != 0) {
		hide_widgets();
		return;
	}

	fd = open(NOTIFIER_UPDATE_COUNT_TMP, O_RDONLY);
	if (fd < 0) // open error
	{
		hide_widgets();
		return;
	}

	int n = read(fd, szReadBuf, sizeof(szReadBuf)); // Value will be short.
	if (n < 0) {
		hide_widgets();
		close(fd);
		return;
	}
	close(fd);

	g_UpdateCount = atoi(szReadBuf);

	if (g_UpdateCount > 0) {
		show_widgets(0);
	}
	// Not registered.
	else if (g_UpdateCount == -1) {
		show_widgets(-1);
	}
	// Off notifier option.
	else if (g_UpdateCount == -99) {
		hide_widgets();
	} else {
		hide_widgets();
	}
}

int main(int argc, char ** argv) {
	gtk_init(&argc, &argv);
	mainIcon = NULL;
	if (signal(SIGUSR1, sig_ChangeStatus) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", NOTIFIER_GUI_EXE_FILE, strerror(errno));
		exit(1);
	}
	

	//set the locale stuff 
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, "/usr/share/locale");
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	

	//Don't uses below code,  This function will works only root.  Remember it! 
	//system(CHECK_NOTIFIER_DAEMON);
	sig_ChangeStatus(SIGUSR1);

	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", NOTIFIER_GUI_EXE_FILE, strerror(errno));
		exit(1);
	}
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", NOTIFIER_GUI_EXE_FILE, strerror(errno));
		exit(1);
	}
	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", NOTIFIER_GUI_EXE_FILE, strerror(errno));
		exit(1);
	}
	if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", NOTIFIER_GUI_EXE_FILE, strerror(errno));
		exit(1);
	}

	int fdNULL;
	fdNULL = open("/dev/null", O_RDWR);
	if (fdNULL == -1) {
		fprintf(stdout, "%s: failed to open /dev/null\n", NOTIFIER_GUI_EXE_FILE);
		exit(1);
	}

	if (dup2(fdNULL, STDERR_FILENO) != STDERR_FILENO) {
		close(fdNULL);
		fprintf(stdout, "%s: failed to duplicate file descriptor\n",
				NOTIFIER_GUI_EXE_FILE);
		exit(1);
	}

	if (fdNULL != STDERR_FILENO)
		close(fdNULL);

	gtk_main();

	return 0;
}

