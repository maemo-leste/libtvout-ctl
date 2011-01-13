/*
 * Maemo TV out control
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

#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xvlib.h>

#include <glib.h>

#include "tvout-ctl.h"

enum {
  ATTR_ENABLE,
  ATTR_TV_STD,
  ATTR_ASPECT,
  ATTR_SCALE,
  NUM_ATTRS,
};

static const char *atom_names[NUM_ATTRS] = {
  [ATTR_ENABLE] = "XV_OMAP_CLONE_TO_TVOUT",
  [ATTR_TV_STD] = "XV_OMAP_TVOUT_STANDARD",
  [ATTR_ASPECT] = "XV_OMAP_TVOUT_WIDESCREEN",
  [ATTR_SCALE ] = "XV_OMAP_TVOUT_SCALE",
};

struct _TVoutCtl {
  Display *dpy;
  XvPortID port;
  int event_base;
  GIOChannel *io;
  guint watch;
  Atom atoms[NUM_ATTRS];
  int values[NUM_ATTRS];
  gpointer ui_data;
};

static gboolean xv_init (TVoutCtl *ctl)
{
  Display *dpy;
  unsigned int version, revision, request_base, event_base, error_base;
  XvAdaptorInfo *adaptors;
  unsigned int num_adaptors;
  unsigned int adaptor_idx;
  int found = 0;
  int r;
  XvPortID port;

  dpy = XOpenDisplay (NULL);
  if (!dpy)
    return FALSE;

  r = XvQueryExtension (dpy, &version, &revision, &request_base, &event_base, &error_base);
  if (r != Success){
    XCloseDisplay (dpy);
    return FALSE;
  }

  r = XvQueryAdaptors (dpy, DefaultRootWindow (dpy), &num_adaptors, &adaptors);
  if (r != Success) {
    XCloseDisplay (dpy);
    return FALSE;
  }

  for (adaptor_idx = 0; adaptor_idx < num_adaptors; adaptor_idx++) {
    unsigned int port_idx;

    for (port_idx = 0; port_idx < adaptors[adaptor_idx].num_ports; port_idx++) {
      XvAttribute *attrs;
      int num_attrs;
      int attr_idx;

      port = adaptors[adaptor_idx].base_id + port_idx;

      attrs = XvQueryPortAttributes (dpy, port, &num_attrs);
      if (!attrs)
        continue;

      found = 0;

      for (attr_idx = 0; attr_idx < num_attrs; attr_idx++) {
        int atom_idx;

        for (atom_idx = 0; atom_idx < NUM_ATTRS; atom_idx++) {
          if (!strcmp (attrs[attr_idx].name, atom_names[atom_idx])) {
            found++;
            break;
          }
        }
      }

      XFree (attrs);

      if (found == NUM_ATTRS)
        break;
    }

    if (found == NUM_ATTRS)
      break;
  }

  XvFreeAdaptorInfo (adaptors);

  if (found != NUM_ATTRS) {
    XCloseDisplay (dpy);
    return FALSE;
  }

  ctl->dpy = dpy;
  ctl->event_base = event_base;
  ctl->port = port;

  return TRUE;
}

static void xv_exit (TVoutCtl *ctl)
{
  XCloseDisplay (ctl->dpy);
}

static void update_ui (TVoutCtl *ctl, int attr_idx, int value)
{
  switch (attr_idx) {
  case ATTR_ENABLE:
    tvout_ui_set_enable (ctl->ui_data, value);
    break;
  case ATTR_TV_STD:
    tvout_ui_set_tv_std (ctl->ui_data, value);
    break;
  case ATTR_ASPECT:
    tvout_ui_set_aspect (ctl->ui_data, value);
    break;
  case ATTR_SCALE:
    tvout_ui_set_scale (ctl->ui_data, value);
    break;
  }
}

static gboolean xv_io_func (GIOChannel *source,
                            GIOCondition condition,
                            gpointer data)
{
  TVoutCtl *ctl = data;
  XEvent e;
  XvPortNotifyEvent *notify = (XvPortNotifyEvent *)&e;
  int attr_idx, value, r;

  while (XPending (ctl->dpy)) {
    XNextEvent (ctl->dpy, &e);

    if (e.type != ctl->event_base + XvPortNotify)
      continue;

    for (attr_idx = 0; attr_idx < NUM_ATTRS; attr_idx++) {
      if (notify->attribute != ctl->atoms[attr_idx])
        continue;

      /*
       * X server bug:
       * Port notifications will be sent even if the
       * server rejected the value and notify->value
       * will contain the invalid value from the client's
       * rejected request. So double check the real
       * situation with XvGetPortAttribute() instead
       * of trusting notify->value implicitly.
       */
      if (notify->value == ctl->values[attr_idx])
        break;

      r = XvGetPortAttribute (ctl->dpy, ctl->port,
                              ctl->atoms[attr_idx], &value);
      if (r != Success)
        break;

      if (ctl->values[attr_idx] == value)
        break;

      ctl->values[attr_idx] = value;
      update_ui (ctl, attr_idx, ctl->values[attr_idx]);
      break;
    }
  }

  return TRUE;
}

static gboolean xv_events_init (TVoutCtl *ctl)
{
  GIOStatus s;
  int r;

  ctl->io = g_io_channel_unix_new (ConnectionNumber (ctl->dpy));
  if (!ctl->io)
    return FALSE;

  s = g_io_channel_set_encoding (ctl->io, NULL, NULL);
  if (s != G_IO_STATUS_NORMAL) {
    g_io_channel_unref (ctl->io);
    return FALSE;
  }

  g_io_channel_set_buffered (ctl->io, FALSE);

  ctl->watch = g_io_add_watch (ctl->io, G_IO_IN | G_IO_PRI, xv_io_func, ctl);
  if (!ctl->watch) {
    g_io_channel_unref (ctl->io);
    return FALSE;
  }

  r = XvSelectPortNotify (ctl->dpy, ctl->port, TRUE);
  if (r != Success) {
    g_source_remove (ctl->watch);
    g_io_channel_unref (ctl->io);
    return FALSE;
  }

  return TRUE;
}

static void xv_events_exit (TVoutCtl *ctl)
{
  XvSelectPortNotify (ctl->dpy, ctl->port, FALSE);
  g_source_remove (ctl->watch);
  g_io_channel_unref (ctl->io);
}

static gboolean xv_update_attributes (TVoutCtl *ctl)
{
  int r;
  int attr_idx;

  r = XInternAtoms (ctl->dpy, (char **)atom_names, NUM_ATTRS, TRUE, ctl->atoms);
  if (!r)
    return FALSE;

  for (attr_idx = 0; attr_idx < NUM_ATTRS; attr_idx++) {
    r = XvGetPortAttribute (ctl->dpy, ctl->port,
                            ctl->atoms[attr_idx],
                            &ctl->values[attr_idx]);
    if (r != Success)
      return FALSE;
  }

  return TRUE;
}

TVoutCtl *tvout_ctl_init (gpointer ui_data)
{
  int r;
  TVoutCtl *ctl;

  ctl = malloc (sizeof *ctl);
  if (!ctl)
    return NULL;

  if (!xv_init (ctl)) {
    free (ctl);
    return NULL;
  }

  if (!xv_events_init (ctl)) {
    xv_exit (ctl);
    free (ctl);
    return NULL;
  }

  if (!xv_update_attributes (ctl)) {
    xv_events_exit (ctl);
    xv_exit (ctl);
    free (ctl);
    return NULL;
  }

  ctl->ui_data = ui_data;

  return ctl;
}

void tvout_ctl_exit (TVoutCtl *ctl)
{
  if (!ctl)
    return;

  xv_events_exit (ctl);
  xv_exit (ctl);
  free (ctl);
}

static void xv_set_attribute (TVoutCtl *ctl, int attr_idx, int value)
{
  int r;

  if (!ctl)
    return;

  if (value == ctl->values[attr_idx])
    return;

  r = XvSetPortAttribute (ctl->dpy, ctl->port, ctl->atoms[attr_idx], value);
  if (r != Success)
    return;

  xv_io_func (NULL, 0, ctl);
}

void tvout_ctl_set_enable (TVoutCtl *ctl, int value)
{
  xv_set_attribute (ctl, ATTR_ENABLE, value);
}

void tvout_ctl_set_tv_std (TVoutCtl *ctl, int value)
{
  xv_set_attribute (ctl, ATTR_TV_STD, value);
}

void tvout_ctl_set_aspect (TVoutCtl *ctl, int value)
{
  xv_set_attribute (ctl, ATTR_ASPECT, value);
}

void tvout_ctl_set_scale (TVoutCtl *ctl, int value)
{
  xv_set_attribute (ctl, ATTR_SCALE, value);
}

static int get_attribute (TVoutCtl *ctl, int attr_idx)
{
  if (!ctl)
    return 0;

  return ctl->values[attr_idx];
}

int tvout_ctl_get_enable (TVoutCtl *ctl)
{
  return get_attribute (ctl, ATTR_ENABLE);
}

int tvout_ctl_get_tv_std (TVoutCtl *ctl)
{
  return get_attribute (ctl, ATTR_TV_STD);
}

int tvout_ctl_get_aspect (TVoutCtl *ctl)
{
  return get_attribute (ctl, ATTR_ASPECT);
}

int tvout_ctl_get_scale (TVoutCtl *ctl)
{
  return get_attribute (ctl, ATTR_SCALE);
}
