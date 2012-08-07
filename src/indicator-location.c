
#include <glib/gi18n.h>

#include <libappindicator/app-indicator.h>

#include <geoclue/geoclue-master.h>
#include <geoclue/geoclue-master-client.h>

/* Base variables */
AppIndicator * indicator = NULL;
GMainLoop * mainloop = NULL;

/* Menu items */
GtkMenuItem * accuracy_item = NULL;
GtkMenuItem * details_item = NULL;

gboolean has_location_details = FALSE;
GtkMenuItem * lat_item = NULL;
GtkMenuItem * lon_item = NULL;
GtkMenuItem * alt_item = NULL;

GtkMenuItem * detail_sep_item = NULL;

gboolean has_address_details = FALSE;
GtkMenuItem * ccode_item = NULL;
GtkMenuItem * country_item = NULL;
GtkMenuItem * region_item = NULL;
GtkMenuItem * locality_item = NULL;
GtkMenuItem * area_item = NULL;
GtkMenuItem * postcode_item = NULL;
GtkMenuItem * street_item = NULL;

/* Geoclue trackers */
static GeoclueMasterClient * geo_master = NULL;
static GeoclueAddress * geo_address = NULL;

/* Prototypes */
static void geo_client_invalid (GeoclueMasterClient * client, gpointer user_data);
static void geo_address_change (GeoclueMasterClient * client, gchar * a, gchar * b, gchar * c, gchar * d, gpointer user_data);
static void geo_create_client (GeoclueMaster * master, GeoclueMasterClient * client, gchar * path, GError * error, gpointer user_data);


/* Update accuracy */
static void
update_accuracy (GeoclueAccuracyLevel level)
{
	const char * icon = NULL;
	const char * icon_desc = NULL;
	const char * item_text = NULL;
	gboolean details_sensitive = TRUE;
	
	switch (level) {
	case GEOCLUE_ACCURACY_LEVEL_NONE:
		icon = "indicator-location-unknown";
		icon_desc = _("Location accuracy unknown");
		item_text = _("Accuracy: Unknown");
		details_sensitive = FALSE;
		break;
	case GEOCLUE_ACCURACY_LEVEL_COUNTRY:
	case GEOCLUE_ACCURACY_LEVEL_REGION:
	case GEOCLUE_ACCURACY_LEVEL_LOCALITY:
		icon = "indicator-location-region";
		icon_desc = _("Location regional accuracy");
		item_text = _("Accuracy: Regional");
		details_sensitive = TRUE;
		break;
	case GEOCLUE_ACCURACY_LEVEL_POSTALCODE:
	case GEOCLUE_ACCURACY_LEVEL_STREET:
		icon = "indicator-location-neighborhood";
		icon_desc = _("Location neighborhood accuracy");
		item_text = _("Accuracy: Neighborhood");
		details_sensitive = TRUE;
		break;
	case GEOCLUE_ACCURACY_LEVEL_DETAILED:
		icon = "indicator-location-specific";
		icon_desc = _("Location specific accuracy");
		item_text = _("Accuracy: Detailed");
		details_sensitive = TRUE;
		break;
	default:
		g_assert_not_reached();
	}

	if (indicator != NULL) {
		app_indicator_set_icon_full(indicator, icon, icon_desc);
	}

	if (accuracy_item != NULL) {
		gtk_menu_item_set_label(accuracy_item, item_text);
	}

	if (details_item != NULL) {
		gtk_widget_set_sensitive(GTK_WIDGET(details_item), details_sensitive);
	}

	return;
}

struct {
	const gchar * hash_value;
	const gchar * item_label;
	GtkMenuItem ** item;
} address_detail_table[] = {
	{GEOCLUE_ADDRESS_KEY_COUNTRYCODE,  N_("Country Code: %s"),  &ccode_item},
	{GEOCLUE_ADDRESS_KEY_COUNTRY,      N_("Country: %s"),       &country_item},
	{GEOCLUE_ADDRESS_KEY_REGION,       N_("Region: %s"),        &region_item},
	{GEOCLUE_ADDRESS_KEY_LOCALITY,     N_("Locality: %s"),      &locality_item},
	{GEOCLUE_ADDRESS_KEY_AREA,         N_("Area: %s"),          &area_item},
	{GEOCLUE_ADDRESS_KEY_POSTALCODE,   N_("Zip Code: %s"),      &postcode_item},
	{GEOCLUE_ADDRESS_KEY_STREET,       N_("Street: %s"),        &street_item},
	{NULL, NULL, NULL}
};

void
update_address_details (GHashTable * details)
{
	int i;
	has_address_details = FALSE;

	for (i = 0; address_detail_table[i].hash_value != NULL; i++) {
		if (g_hash_table_contains(details, address_detail_table[i].hash_value)) {
			gchar * string = g_strdup_printf(_(address_detail_table[i].item_label), g_hash_table_lookup(details, address_detail_table[i].hash_value));
			gtk_menu_item_set_label(*address_detail_table[i].item, string);
			g_free(string);

			gtk_widget_show(GTK_WIDGET(*address_detail_table[i].item));

			has_address_details = TRUE;
		} else {
			gtk_widget_hide(GTK_WIDGET(*address_detail_table[i].item));
		}
	}

	if (has_location_details && has_address_details) {
		gtk_widget_show(GTK_WIDGET(detail_sep_item));
	} else {
		gtk_widget_hide(GTK_WIDGET(detail_sep_item));
	}

	return;
}

/* Callback from getting the address */
static void
geo_address_cb (GeoclueAddress * address, int timestamp, GHashTable * addy_data, GeoclueAccuracy * accuracy, GError * error, gpointer user_data)
{
	if (error != NULL) {
		g_warning("Unable to get Geoclue address: %s", error->message);
		g_clear_error (&error);
		return;
	}

	GeoclueAccuracyLevel level = GEOCLUE_ACCURACY_LEVEL_NONE;
	geoclue_accuracy_get_details(accuracy, &level, NULL, NULL);
	update_accuracy(level);

	update_address_details(addy_data);

	return;
}

/* Clean up the reference we kept to the address and make sure to
   drop the signals incase someone else has one. */
static void
geo_address_clean (void)
{
	if (geo_address == NULL) {
		return;
	}

	g_signal_handlers_disconnect_by_func(G_OBJECT(geo_address), geo_address_cb, NULL);
	g_object_unref(G_OBJECT(geo_address));

	geo_address = NULL;

	return;
}

/* Clean up and remove all signal handlers from the client as we
   unreference it as well. */
static void
geo_client_clean (void)
{
	if (geo_master == NULL) {
		return;
	}

	g_signal_handlers_disconnect_by_func(G_OBJECT(geo_master), geo_client_invalid, NULL);
	g_signal_handlers_disconnect_by_func(G_OBJECT(geo_master), geo_address_change, NULL);
	g_object_unref(G_OBJECT(geo_master));

	geo_master = NULL;

	return;
}

/* Callback from creating the address */
static void
geo_create_address (GeoclueMasterClient * master, GeoclueAddress * address, GError * error, gpointer user_data)
{
	if (error != NULL) {
		g_warning("Unable to create GeoClue address: %s", error->message);
		g_clear_error (&error);
		return;
	}

	/* We shouldn't have created a new address if we already had one
	   so this is a warning.  But, it really is only a mem-leak so we
	   don't need to error out. */
	g_warn_if_fail(geo_address == NULL);
	geo_address_clean();

	g_debug("Created Geoclue Address");
	geo_address = address;
	g_object_ref(G_OBJECT(geo_address));

	geoclue_address_get_address_async(geo_address, geo_address_cb, NULL);

	g_signal_connect(G_OBJECT(address), "address-changed", G_CALLBACK(geo_address_cb), NULL);

	return;
}

/* Callback from setting requirements */
static void
geo_req_set (GeoclueMasterClient * master, GError * error, gpointer user_data)
{
	if (error != NULL) {
		g_warning("Unable to set Geoclue requirements: %s", error->message);
		g_clear_error (&error);
	}
	return;
}

/* Client is killing itself rather oddly */
static void
geo_client_invalid (GeoclueMasterClient * client, gpointer user_data)
{
	g_warning("Master client invalid, rebuilding.");

	/* Client changes we can assume the address is now invalid so we
	   need to unreference the one we had. */
	geo_address_clean();

	/* And our master client is invalid */
	geo_client_clean();

	GeoclueMaster * master = geoclue_master_get_default();
	geoclue_master_create_client_async(master, geo_create_client, NULL);

	update_accuracy(GEOCLUE_ACCURACY_LEVEL_NONE);

	return;
}

/* Address provider changed, we need to get that one */
static void
geo_address_change (GeoclueMasterClient * client, gchar * a, gchar * b, gchar * c, gchar * d, gpointer user_data)
{
	g_warning("Address provider changed.  Let's change");

	/* If the address is supposed to have changed we need to drop the old
	   address before starting to get the new one. */
	geo_address_clean();

	geoclue_master_client_create_address_async(geo_master, geo_create_address, NULL);

	update_accuracy(GEOCLUE_ACCURACY_LEVEL_NONE);

	return;
}

/* Callback from creating the client */
static void
geo_create_client (GeoclueMaster * master, GeoclueMasterClient * client, gchar * path, GError * error, gpointer user_data)
{
	g_debug("Created Geoclue client at: %s", path);

	geo_master = client;

	if (error != NULL) {
		g_warning("Unable to get a GeoClue client!  '%s'  Geolocation based timezone support will not be available.", error->message);
		g_clear_error (&error);
		return;
	}

	if (geo_master == NULL) {
		g_warning(_("Unable to get a GeoClue client!  Geolocation based timezone support will not be available."));
		return;
	}

	g_object_ref(G_OBJECT(geo_master));

	/* New client, make sure we don't have an address hanging on */
	geo_address_clean();

	geoclue_master_client_set_requirements_async(geo_master,
	                                             GEOCLUE_ACCURACY_LEVEL_REGION,
	                                             0,
	                                             FALSE,
	                                             GEOCLUE_RESOURCE_ALL,
	                                             geo_req_set,
	                                             NULL);

	geoclue_master_client_create_address_async(geo_master, geo_create_address, NULL);

	g_signal_connect(G_OBJECT(client), "invalidated", G_CALLBACK(geo_client_invalid), NULL);
	g_signal_connect(G_OBJECT(client), "address-provider-changed", G_CALLBACK(geo_address_change), NULL);

	return;
}

void
open_maps (void)
{
	g_spawn_command_line_async("emerillon", NULL);
	return;
}

void
open_debuglocation (void)
{
	g_spawn_command_line_async("geoclue-test-gui", NULL);
	return;
}

GtkWidget *
build_details_items (void)
{
	GtkWidget * menu = gtk_menu_new();

	lat_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(lat_item));
	gtk_widget_set_sensitive(GTK_WIDGET(lat_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(lat_item));

	lon_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(lon_item));
	gtk_widget_set_sensitive(GTK_WIDGET(lon_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(lon_item));

	alt_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(alt_item));
	gtk_widget_set_sensitive(GTK_WIDGET(alt_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(alt_item));

	detail_sep_item = GTK_MENU_ITEM(gtk_separator_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(alt_item));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(alt_item));

	ccode_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(ccode_item));
	gtk_widget_set_sensitive(GTK_WIDGET(ccode_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(ccode_item));

	country_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(country_item));
	gtk_widget_set_sensitive(GTK_WIDGET(country_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(country_item));

	region_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(region_item));
	gtk_widget_set_sensitive(GTK_WIDGET(region_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(region_item));

	locality_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(locality_item));
	gtk_widget_set_sensitive(GTK_WIDGET(locality_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(locality_item));

	area_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(area_item));
	gtk_widget_set_sensitive(GTK_WIDGET(area_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(area_item));

	postcode_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(postcode_item));
	gtk_widget_set_sensitive(GTK_WIDGET(postcode_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(postcode_item));

	street_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_hide(GTK_WIDGET(street_item));
	gtk_widget_set_sensitive(GTK_WIDGET(street_item), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(street_item));

	return menu;
}

void
build_indicator (void)
{
	indicator = app_indicator_new_with_path("indicator-location", "indicator-location-unknown", APP_INDICATOR_CATEGORY_SYSTEM_SERVICES, ICON_DIR);
	app_indicator_set_title(indicator, _("Location"));
	app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

	GtkMenu * menu = GTK_MENU(gtk_menu_new());

	accuracy_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_show(GTK_WIDGET(accuracy_item));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(accuracy_item));
	gtk_widget_set_sensitive(GTK_WIDGET(accuracy_item), FALSE);

	details_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Details")));
	gtk_widget_show(GTK_WIDGET(details_item));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(details_item));
	gtk_widget_set_sensitive(GTK_WIDGET(details_item), FALSE);

	gtk_menu_item_set_submenu(details_item, build_details_items());

	gchar * maps_in_path = g_find_program_in_path("emerillon");
	if (maps_in_path != NULL) {
		g_free(maps_in_path);
		maps_in_path = NULL;

		GtkWidget * sep = gtk_separator_menu_item_new();
		gtk_widget_show(sep);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(sep));

		GtkWidget * maps = gtk_menu_item_new_with_label(_("Open Map…"));
		gtk_widget_show(maps);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(maps));
		g_signal_connect(G_OBJECT(maps), "activate", G_CALLBACK(open_maps), NULL);
	}

	gchar * geoclue_in_path = g_find_program_in_path("geoclue-test-gui");
	if (geoclue_in_path != NULL) {
		g_free(geoclue_in_path);
		geoclue_in_path = NULL;

		GtkWidget * sep = gtk_separator_menu_item_new();
		gtk_widget_show(sep);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(sep));

		GtkWidget * debugloc = gtk_menu_item_new_with_label(_("Debug Location…"));
		gtk_widget_show(debugloc);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(debugloc));
		g_signal_connect(G_OBJECT(debugloc), "activate", G_CALLBACK(open_debuglocation), NULL);
	}

	gtk_widget_show(GTK_WIDGET(menu));

	app_indicator_set_menu(indicator, menu);

	update_accuracy(GEOCLUE_ACCURACY_LEVEL_NONE);

	return;
}

int
main (int argc, char * argv[])
{
	gtk_init(&argc, &argv);

	/* Setup geoclue */
	GeoclueMaster * master = geoclue_master_get_default();
	geoclue_master_create_client_async(master, geo_create_client, NULL);

	build_indicator();

	/* Mainloop */
	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	/* clean up */
	g_main_loop_unref(mainloop);
	mainloop = NULL;

	geo_address_clean();
	geo_client_clean();

	return 0;
}
