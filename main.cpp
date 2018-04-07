//
// Copyright (c) 2016-2018, Karbo developers
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <cstdio>
#include <string.h>
#include <panel-applet.h>
#include <gtk/gtklabel.h>

#include "MarketsAPI.h"


static gint update_lable_applet(gpointer label){
  char buffer[16];
  char result[64];
  const char target_pair[] = "KRB/UAH";
  const char sep[] = "/";
  const char sub_a[] = ": ";
  Markets markets = Markets();
  markets.doLoad();
  memset(result, 0, 64);
  if (markets.getStatus()){
    if (markets.getTickerByName(target_pair)){
      memset(buffer, 0, 16);
      strcat(result, target_pair);
      strcat(result, sub_a);
      sprintf(buffer, "%.4f", markets.ticker.sell_active);
      strcat(result, buffer);
      memset(buffer, 0, 16);
      strcat(result, sep);
      sprintf(buffer, "%.4f", markets.ticker.buy_active);
      strcat(result, buffer);
    }
  }
  gtk_label_set_text(GTK_LABEL(label), result);
  return 1;
}

static void display_about_dialog(BonoboUIComponent *uic, struct MultiRes *applet){
  gtk_show_about_dialog (NULL,
                         "version", "1.0",
                         "copyright", "Copyright \xC2\xA9 2018 Lastick",
                         "comments", _("Апплет Karbo-Markets для среды Gnome2"),
                         "authors", NULL,
                         "documenters", NULL,
                         "translator-credits", _("translator-credits"),
                         "logo_icon_name", NULL,
                         NULL);
}

static const char Context_menu_xml[] =
   "<popup name=\"button3\">\n"
   "  <menuitem name=\"About Item\" "
   "    verb=\"About\" "
   "    _label=\"_About...\"\n"
   "    pixtype=\"stock\" "
   "    pixname=\"gnome-stock-about\"/>\n"
   "</popup>\n";

static const BonoboUIVerb applet_menu_verbs[] = {
  BONOBO_UI_UNSAFE_VERB ("About", display_about_dialog),
  BONOBO_UI_VERB_END
};

static gboolean karbo_ticker_applet_fill(PanelApplet *applet, const gchar *iid, gpointer data){
  GtkWidget *label;
  if (strcmp(iid, "OAFIID:KarboTickerApplet") != 0){
    return FALSE;
  }
  panel_applet_setup_menu(applet, Context_menu_xml, applet_menu_verbs, applet);
  label = gtk_label_new("Loading...");
  gtk_container_add(GTK_CONTAINER(applet), label);
  gtk_widget_show_all(GTK_WIDGET(applet));
  gtk_timeout_add(10000, update_lable_applet, label);
  return TRUE;
}

int main(int argc, char *argv[]){
  GOptionContext *context;
  GError *error;
  int retval;
  _PANEL_APPLET_SETUP_GETTEXT(TRUE);
  context = g_option_context_new("");
  g_option_context_add_group(context, gtk_get_option_group(TRUE));
  g_option_context_add_group(context, bonobo_activation_get_goption_group());
  error = NULL;
  if (!g_option_context_parse(context, &argc, &argv, &error)){
    if (error){
      g_printerr ("Cannot parse arguments: %s.\n", error->message);
      g_error_free (error);
    } else {
      g_printerr("Cannot parse arguments.\n");
    }
    g_option_context_free(context);
    return 1;
  }
  gtk_init(&argc, &argv);
  if (!bonobo_init(&argc, argv)){
    g_printerr ("Cannot initialize bonobo.\n");
    return 1;
  }
  retval = panel_applet_factory_main("OAFIID:KarboTickerApplet_Factory", PANEL_TYPE_APPLET, karbo_ticker_applet_fill, NULL);
  g_option_context_free(context);
  return retval;
}
