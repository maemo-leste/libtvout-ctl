/*
 * Maemo TV out control home plugin
 * Copyright (C) 2010  Ville Syrjälä <syrjala@sci.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <hildon/hildon.h>

#include "tvout-home-plugin.h"
#include "tvout-ctl.h"

HD_DEFINE_PLUGIN_MODULE (TVoutHomePlugin, tvout_home_plugin, HD_TYPE_HOME_PLUGIN_ITEM)

#define TVOUT_HOME_PLUGIN_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TVOUT_TYPE_HOME_PLUGIN, TVoutHomePluginPrivate))

struct _TVoutHomePluginPrivate
{
  TVoutCtl *tvout_ctl;

  GtkWidget *enable_button;
  GtkWidget *tv_std_button;
  GtkWidget *aspect_button;
  GtkWidget *scale_dec_button;
  GtkWidget *scale_inc_button;
  GtkWidget *scale_label;
};

void tvout_ui_set_enable (gpointer data, gint value)
{
  static const gchar *labels[] = {
    "OFF",
    "ON",
  };
  TVoutHomePluginPrivate *priv = data;

  if (value < 0 || value > 1)
    return;

  gtk_button_set_label (GTK_BUTTON (priv->enable_button), labels[value]);
}

void tvout_ui_set_tv_std (gpointer data, gint value)
{
  static const gchar *labels[] = {
    "PAL",
    "NTSC",
  };
  TVoutHomePluginPrivate *priv = data;

  if (value < 0 || value > 1)
    return;

  gtk_button_set_label (GTK_BUTTON (priv->tv_std_button), labels[value]);
}

void tvout_ui_set_aspect (gpointer data, gint value)
{
  static const gchar *labels[] = {
    "4:3",
    "16:9",
  };
  TVoutHomePluginPrivate *priv = data;

  if (value < 0 || value > 1)
    return;

  gtk_button_set_label (GTK_BUTTON (priv->aspect_button), labels[value]);
}

void tvout_ui_set_scale (gpointer data, gint value)
{
  TVoutHomePluginPrivate *priv = data;
  gchar text[4];

  if (value < 1 || value > 100)
    return;

  g_snprintf(text, sizeof text, "%d", value);
  gtk_label_set_text (GTK_LABEL (priv->scale_label), text);

  gtk_widget_set_sensitive (priv->scale_dec_button, value > 1);
  gtk_widget_set_sensitive (priv->scale_inc_button, value < 100);
}

static void enable_button_clicked (GtkButton *widget,
                                   gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;

  tvout_ctl_set_enable (priv->tvout_ctl, !tvout_ctl_get_enable (priv->tvout_ctl));
}

static void tv_std_button_clicked (GtkButton *widget,
                                   gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;

  tvout_ctl_set_tv_std (priv->tvout_ctl, !tvout_ctl_get_tv_std (priv->tvout_ctl));
}

static void aspect_button_clicked (GtkButton *widget,
                                   gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;

  tvout_ctl_set_aspect (priv->tvout_ctl, !tvout_ctl_get_aspect (priv->tvout_ctl));
}

static void scale_dec_button_clicked (GtkButton *widget,
                                      gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;
  int value;

  value = tvout_ctl_get_scale (priv->tvout_ctl) - 1;
  if (value < 1 || value > 100)
    return;

  tvout_ctl_set_scale (priv->tvout_ctl, value);
}

static void scale_inc_button_clicked (GtkButton *widget,
                                      gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;
  int value;

  value = tvout_ctl_get_scale (priv->tvout_ctl) + 1;
  if (value < 1 || value > 100)
    return;

  tvout_ctl_set_scale (priv->tvout_ctl, value);
}

static GtkWidget *create_ui_enable (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("TV out");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->enable_button = button = gtk_button_new ();

  gtk_button_set_label (GTK_BUTTON (button), "OFF");
  gtk_button_set_label (GTK_BUTTON (button), "ON");

  tvout_ui_set_enable (priv, tvout_ctl_get_enable (priv->tvout_ctl));

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (enable_button_clicked), priv);

  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui_tv_std (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("Video format");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->tv_std_button = button = gtk_button_new ();

  gtk_button_set_label (GTK_BUTTON (button), "PAL");
  gtk_button_set_label (GTK_BUTTON (button), "NTSC");

  tvout_ui_set_tv_std (priv, tvout_ctl_get_tv_std (priv->tvout_ctl));

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (tv_std_button_clicked), priv);

  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui_aspect (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("Aspect ratio");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->aspect_button = button = gtk_button_new ();

  gtk_button_set_label (GTK_BUTTON (button), "4:3");
  gtk_button_set_label (GTK_BUTTON (button), "16:9");

  tvout_ui_set_aspect (priv, tvout_ctl_get_aspect (priv->tvout_ctl));

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (aspect_button_clicked), priv);

  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui_scale (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("Scale");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->scale_dec_button = button = gtk_button_new_with_label ("<");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (scale_dec_button_clicked), priv);
  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  priv->scale_label = label = gtk_label_new ("0");
  tvout_ui_set_scale (priv, tvout_ctl_get_scale (priv->tvout_ctl));
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->scale_inc_button = button = gtk_button_new_with_label (">");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (scale_inc_button_clicked), priv);
  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui (TVoutHomePlugin *self)
{
  TVoutHomePluginPrivate *priv = self->priv;
  GtkWidget *vbox, *widget;

  vbox = gtk_vbox_new (0, FALSE);

  widget = create_ui_enable (priv);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);

  widget = create_ui_tv_std (priv);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);

  widget = create_ui_aspect (priv);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);

  widget = create_ui_scale (priv);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);

  return vbox;
}

static void
tvout_home_plugin_init (TVoutHomePlugin *self)
{
  TVoutHomePluginPrivate *priv;
  GtkWidget *contents;

  self->priv = priv = TVOUT_HOME_PLUGIN_GET_PRIVATE (self);

  priv->tvout_ctl = tvout_ctl_init (priv);

  contents = create_ui (self);

  if (!priv->tvout_ctl)
    gtk_widget_set_sensitive (contents, FALSE);

  gtk_widget_show_all (contents);

  gtk_container_add (GTK_CONTAINER (self), contents);
}

static void tvout_home_plugin_finalize (GObject *self)
{
  TVoutHomePluginPrivate *priv = TVOUT_HOME_PLUGIN (self)->priv;

  tvout_ctl_exit (priv->tvout_ctl);

  G_OBJECT_CLASS (tvout_home_plugin_parent_class)->finalize (self);
}

static void
tvout_home_plugin_class_finalize (TVoutHomePluginClass *klass)
{
}

static void
tvout_home_plugin_class_init (TVoutHomePluginClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = tvout_home_plugin_finalize;

  g_type_class_add_private (klass, sizeof (TVoutHomePluginPrivate));
}
