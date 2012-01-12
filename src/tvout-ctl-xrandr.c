/*
 * Maemo TV out control
 * Copyright (C) 2011-2012  Ville Syrjälä <syrjala@sci.fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/extensions/Xrandr.h>

#include "tvout-ctl.h"

typedef struct {
	Atom atom;
	Atom type;

	long value;
	XRRPropertyInfo *info;
} RRProp;

enum {
	PROP_SIGNAL_FORMAT,
	PROP_SIGNAL_PROPERTIES,
	PROP_TV_ASPECT_RATIO,
	PROP_TV_SCALE,
	PROP_TV_DYNAMIC_ASPECT_RATIO,
	PROP_TV_X_OFFSET,
	PROP_TV_Y_OFFSET,
	PROP_XV_CLONE_FULLSCREEN,
	NUM_PROPS,
};

static const char *prop_names[NUM_PROPS] = {
	[PROP_SIGNAL_FORMAT] = RR_PROPERTY_SIGNAL_FORMAT,
	[PROP_SIGNAL_PROPERTIES] = RR_PROPERTY_SIGNAL_PROPERTIES,
	[PROP_TV_ASPECT_RATIO] = "TVAspectRatio",
	[PROP_TV_SCALE] = "TVScale",
	[PROP_TV_DYNAMIC_ASPECT_RATIO] = "TVDynamicAspectRatio",
	[PROP_TV_X_OFFSET] = "TVXOffset",
	[PROP_TV_Y_OFFSET] = "TVYOffset",
	[PROP_XV_CLONE_FULLSCREEN] = "XvCloneFullscreen",
};

static const Atom prop_types[NUM_PROPS] = {
	[PROP_SIGNAL_FORMAT] = XA_ATOM,
	[PROP_SIGNAL_PROPERTIES] = XA_ATOM,
	[PROP_TV_ASPECT_RATIO] = XA_ATOM,
	[PROP_TV_SCALE] = XA_INTEGER,
	[PROP_TV_DYNAMIC_ASPECT_RATIO] = XA_INTEGER,
	[PROP_TV_X_OFFSET] = XA_INTEGER,
	[PROP_TV_Y_OFFSET] = XA_INTEGER,
	[PROP_XV_CLONE_FULLSCREEN] = XA_INTEGER,
};

struct _TVoutCtl {
	Display *dpy;
	int event_base;

	RRCrtc crtc;
	RRMode mode;
	RROutput output;
	bool enabled;
	RRProp props[NUM_PROPS];

	TVoutCtlNotify ui_notify;
	void *ui_data;
};

static bool rr_init(TVoutCtl *ctl)
{
	Display *dpy;
	int minor, major, event_base, error_base;

	dpy = XOpenDisplay(NULL);
	if (!dpy)
		return false;

	if (!XRRQueryVersion(dpy, &major, &minor)) {
		XCloseDisplay(dpy);
		return false;
	}

	printf("RandR extension %d.%d\n", major, minor);

	if (!XRRQueryExtension(dpy, &event_base, &error_base)) {
		XCloseDisplay(dpy);
		return false;
	}

	printf("RandR event_base=%d error_base=%d\n", event_base, error_base);

	XRRSelectInput(dpy, DefaultRootWindow(dpy),
		       RROutputChangeNotifyMask | RROutputPropertyNotifyMask);

	ctl->dpy = dpy;
	ctl->event_base = event_base;

	return true;
}

static void rr_exit(TVoutCtl *ctl)
{
	XRRSelectInput(ctl->dpy, DefaultRootWindow(ctl->dpy), 0);
	XCloseDisplay(ctl->dpy);
}

static bool realloc_property_info(RRProp *prop, int num_values)
{
	/* FIXME somewhat questionable to use malloc()+XFree() */
	XRRPropertyInfo *info = malloc(sizeof(XRRPropertyInfo) + num_values * sizeof(long));
	if (!info)
		return false;

	memcpy(info, prop->info, sizeof(XRRPropertyInfo));

	info->num_values = num_values;
	info->values = (long *) (info + 1);

	XFree(prop->info);
	prop->info = info;

	return true;
}

/*
 * The X driver forgot to provide the list of valid
 * values for some properties. Fix it up manually.
 */
static bool fixup_property_info(TVoutCtl *ctl, RRProp *prop, const char *name)
{
	if (strcmp(name, RR_PROPERTY_SIGNAL_FORMAT) == 0) {
		if (!realloc_property_info(prop, 2))
			return false;
		prop->info->values[0] = XInternAtom(ctl->dpy, "Composite-PAL", True);
		prop->info->values[1] = XInternAtom(ctl->dpy, "Composite-NTSC", True);
		if (prop->info->values[0] == None ||
		    prop->info->values[1] == None)
			return false;
	} else if (strcmp(name, RR_PROPERTY_SIGNAL_PROPERTIES) == 0) {
		if (!realloc_property_info(prop, 2))
			return false;
		prop->info->values[0] = XInternAtom(ctl->dpy, "PAL", True);
		prop->info->values[1] = XInternAtom(ctl->dpy, "NTSC", True);
		if (prop->info->values[0] == None ||
		    prop->info->values[1] == None)
			return false;
	} else if (strcmp(name, "TVAspectRatio") == 0) {
		if (!realloc_property_info(prop, 2))
			return false;
		prop->info->values[0] = XInternAtom(ctl->dpy, "4:3", True);
		prop->info->values[1] = XInternAtom(ctl->dpy, "16:9", True);
		if (prop->info->values[0] == None ||
		    prop->info->values[1] == None)
			return false;
	} else
		return false;

	return true;
}

static bool probe_property(TVoutCtl *ctl, RRProp *prop, const char *name)
{
	Atom type;
	int format;
	unsigned long nitems, bytes_after;
	unsigned char *data;
	long value;

	if (XRRGetOutputProperty(ctl->dpy, ctl->output, prop->atom,
				 0, 100, False, False,
				 prop->type, &type, &format,
				 &nitems, &bytes_after, &data) != Success)
		return false;

	/* sanity check */
	if (type != prop->type || format != 32 || nitems != 1) {
		XFree(data);
		return false;
	}

	value = *(long *) data;
	XFree(data);

	prop->info = XRRQueryOutputProperty(ctl->dpy, ctl->output, prop->atom);
	if (!prop->info)
		return false;

	switch (type) {
	case XA_INTEGER:
		/* sanity check */
		if (!prop->info->range || prop->info->num_values != 2) {
			XFree(prop->info);
			prop->info = NULL;
			return false;
		}

		prop->value = value;
		break;
	case XA_ATOM:
		/* sanity check & fixup */
		if (prop->info->range ||
		    (prop->info->num_values == 0 &&
		     !fixup_property_info(ctl, prop, name))) {
			XFree(prop->info);
			prop->info = NULL;
			return false;
		}

		prop->value = value;
		break;
	default:
		XFree(prop->info);
		prop->info = NULL;
		return false;
	}

	return true;
}

static void init_property(TVoutCtl *ctl, Atom atom, const char *name)
{
	RRProp *prop;
	int i;

	for (i = 0; i < NUM_PROPS; i++) {
		prop = &ctl->props[i];

		if (strcmp(prop_names[i], name) != 0)
			continue;

		prop->atom = atom;
		prop->type = prop_types[i];

		break;
	}

	if (i == NUM_PROPS)
		return;

	probe_property(ctl, prop, name);
}

static void free_properties(TVoutCtl *ctl)
{
	int i;

	if (!ctl)
		return;

	for (i = 0; i < NUM_PROPS; i++) {
		RRProp *prop = &ctl->props[i];
		XFree(prop->info);
		prop->info = NULL;
	}
}

static bool init_properties(TVoutCtl *ctl)
{
	int i;
	Atom *props;
	int nprop = 0;
	char **names;

	props = XRRListOutputProperties(ctl->dpy, ctl->output, &nprop);
	if (!props)
		return false;

	names = calloc(nprop, sizeof names[0]);
	if (!names) {
		XFree(props);
		return false;
	}

	XGetAtomNames(ctl->dpy, props, nprop, names);

	for (i = 0; i < nprop; i++) {
		init_property(ctl, props[i], names[i]);
		XFree(names[i]);
	}

	free(names);

	XFree(props);

	/* Did we find them all? */
	for (i = 0; i < NUM_PROPS; i++) {
		if (ctl->props[i].atom == None) {
			free_properties(ctl);
			return false;
		}
	}

	return true;
}

static void update_ui(TVoutCtl *ctl, enum TVoutCtlAttr attr, int value)
{
	if (!ctl->ui_notify)
		return;

	ctl->ui_notify(ctl->ui_data, attr, value);
}

static void handle_output_change(TVoutCtl *ctl,
				 const XRROutputChangeNotifyEvent *e)
{
	bool enabled;

	if (e->output != ctl->output)
		return;

	enabled = e->crtc != 0;

	if (enabled == ctl->enabled)
		return;

	ctl->enabled = enabled;

	update_ui(ctl, TVOUT_CTL_ENABLE, ctl->enabled);
}

static bool update_property(TVoutCtl *ctl, RRProp *prop)
{
	Atom type;
	int format;
	unsigned long nitems, bytes_after;
	unsigned char *data;
	long value;

	if (XRRGetOutputProperty(ctl->dpy, ctl->output, prop->atom,
				 0, 100, False, False,
				 prop->type, &type, &format,
				 &nitems, &bytes_after, &data) != Success)
		return false;

	/* sanity check */
	if (type != prop->type || format != 32 || nitems != 1) {
		XFree(data);
		return false;
	}

	value = *(long *) data;
	XFree(data);

	if (prop->value == value)
		return false;

	prop->value = value;

	return true;
}

static int prop_value_to_attr_value(const RRProp *prop)
{
	int i;

	switch (prop->type) {
	case XA_INTEGER:
		return prop->value;
	case XA_ATOM:
		for (i = 0; i < prop->info->num_values; i++)
			if (prop->value == prop->info->values[i])
				return i;
		return -1;
	default:
		return -1;
	}
}

static void handle_output_property(TVoutCtl *ctl,
				   const XRROutputPropertyNotifyEvent *e)
{
	int i;

	if (e->output != ctl->output)
		return;

	for (i = 0; i < NUM_PROPS; i++) {
		int value;
		RRProp *prop = &ctl->props[i];

		if (e->property != prop->atom)
			continue;

		if (e->state != PropertyNewValue)
			return;

		if (!update_property(ctl, prop))
			return;

		value = prop_value_to_attr_value(prop);
		if (value < 0)
			return;

		update_ui(ctl, i, value);
		return;
	}
}

typedef union {
	XEvent event;
	XRRNotifyEvent notify_event;
	XRROutputChangeNotifyEvent output_change_noitfy_event;
	XRROutputPropertyNotifyEvent output_property_notify_event;
} XRREvent;

static void handle_notify(TVoutCtl *ctl, const XRREvent *rre)
{
	const XRRNotifyEvent *e = &rre->notify_event;

	switch (e->subtype) {
	case RRNotify_OutputChange:
		handle_output_change(ctl, &rre->output_change_noitfy_event);
		break;
	case RRNotify_OutputProperty:
		handle_output_property(ctl, &rre->output_property_notify_event);
		break;
	default:
		printf("Unknown RandR event subtype %d\n", e->subtype);
		break;
	}
}

static void process_events(TVoutCtl *ctl)
{
	XRREvent rre;
	XEvent *e = &rre.event;

	while (XPending(ctl->dpy) > 0) {
		XNextEvent(ctl->dpy, e);

		if (e->type == ctl->event_base + RRNotify)
			handle_notify(ctl, &rre);
	}
}

static bool probe_output(TVoutCtl *ctl,
			 XRRScreenResources *resources,
			 RROutput output)
{
	XRROutputInfo *info;
	bool ret = false;

	info = XRRGetOutputInfo(ctl->dpy, resources, output);
	if (!info)
		return false;

	if (strcmp(info->name, "TV") == 0 &&
	    info->ncrtc == 1 && info->nmode > 0) {
		ctl->output = output;
		ctl->crtc = info->crtcs[0];
		ctl->mode = info->modes[0];
		ctl->enabled = info->crtc != 0;
		ret = true;
	}

	XRRFreeOutputInfo(info);

	return ret;
}

static bool probe_outputs(TVoutCtl *ctl)
{
	XRRScreenResources *resources;
	int i;

	resources = XRRGetScreenResources(ctl->dpy, DefaultRootWindow(ctl->dpy));
	if (!resources)
		return false;

	for (i = 0; i < resources->noutput; i++)
		if (probe_output(ctl, resources, resources->outputs[i]))
			break;

	XRRFreeScreenResources(resources);

	return ctl->output != 0;
}

static Atom index_to_atom(const RRProp *prop, int i)
{
	if (i < 0 || i >= prop->info->num_values)
		return None;

	return prop->info->values[i];
}

static int set_property(TVoutCtl *ctl, int i, long value)
{
	RRProp *prop;

	if (i >= NUM_PROPS)
		return -1;

	prop = &ctl->props[i];

	switch (prop->type) {
	case XA_INTEGER:
		if (value < prop->info->values[0] ||
		    value > prop->info->values[1])
			return -1;

		if (value == prop->value)
			return 0;
		break;
	case XA_ATOM:
		value = (long) index_to_atom(prop, value);
		if (value == None)
			return -1;

		if (value == prop->value)
			return 0;
		break;
	default:
		return -1;
	}

	XRRChangeOutputProperty(ctl->dpy, ctl->output, prop->atom, prop->type,
				32, PropModeReplace, (unsigned char *) &value, 1);

	/* FIXME are we sure to get a notification? */

	process_events(ctl);

	return 0;
}

static int set_crtc_config(TVoutCtl *ctl, int value)
{
	XRRScreenResources *resources;

	if (value != 0 && value != 1)
		return -1;

	resources = XRRGetScreenResourcesCurrent(ctl->dpy, DefaultRootWindow(ctl->dpy));
	if (!resources)
		return -1;

	if (value)
		XRRSetCrtcConfig(ctl->dpy, resources, ctl->crtc, CurrentTime,
				 0, 0, ctl->mode, RR_Rotate_0, &ctl->output, 1);
	else
		XRRSetCrtcConfig(ctl->dpy, resources, ctl->crtc, CurrentTime,
				 0, 0, None, RR_Rotate_0, NULL, 0);

	XRRFreeScreenResources(resources);

	process_events(ctl);

	return 0;
}

static int get_property(const TVoutCtl *ctl, int i)
{
	const RRProp *prop;

	if (i >= NUM_PROPS)
		return -1;

	prop = &ctl->props[i];

	return prop_value_to_attr_value(prop);
}

int tvout_ctl_set(TVoutCtl *ctl, enum TVoutCtlAttr attr, int value)
{
	if (!ctl)
		return -1;

	switch (attr) {
	case TVOUT_CTL_ENABLE:
		return set_crtc_config(ctl, value);
	case TVOUT_CTL_TV_STD:
		return set_property(ctl, PROP_SIGNAL_PROPERTIES, value);
	case TVOUT_CTL_ASPECT:
		return set_property(ctl, PROP_TV_ASPECT_RATIO, value);
	case TVOUT_CTL_SCALE:
		return set_property(ctl, PROP_TV_SCALE, value);
	case TVOUT_CTL_DYNAMIC_ASPECT:
		return set_property(ctl, PROP_TV_DYNAMIC_ASPECT_RATIO, value);
	case TVOUT_CTL_XOFFSET:
		return set_property(ctl, PROP_TV_X_OFFSET, value);
	case TVOUT_CTL_YOFFSET:
		return set_property(ctl, PROP_TV_Y_OFFSET, value);
	case TVOUT_CTL_FULLSCREEN_VIDEO:
		return set_property(ctl, PROP_XV_CLONE_FULLSCREEN, value);
	default:
		return -1;
	}
}

int tvout_ctl_get(TVoutCtl *ctl, enum TVoutCtlAttr attr)
{
	if (!ctl)
		return -1;

	switch (attr) {
	case TVOUT_CTL_ENABLE:
		return ctl->enabled;
	case TVOUT_CTL_TV_STD:
		return get_property(ctl, PROP_SIGNAL_PROPERTIES);
	case TVOUT_CTL_ASPECT:
		return get_property(ctl, PROP_TV_ASPECT_RATIO);
	case TVOUT_CTL_SCALE:
		return get_property(ctl, PROP_TV_SCALE);
	case TVOUT_CTL_DYNAMIC_ASPECT:
		return get_property(ctl, PROP_TV_DYNAMIC_ASPECT_RATIO);
	case TVOUT_CTL_XOFFSET:
		return get_property(ctl, PROP_TV_X_OFFSET);
	case TVOUT_CTL_YOFFSET:
		return get_property(ctl, PROP_TV_Y_OFFSET);
	case TVOUT_CTL_FULLSCREEN_VIDEO:
		return get_property(ctl, PROP_XV_CLONE_FULLSCREEN);
	default:
		return -1;
	}
}

int tvout_ctl_fd(TVoutCtl *ctl)
{
	if (!ctl)
		return -1;

	return ConnectionNumber(ctl->dpy);
}

void tvout_ctl_fd_ready(TVoutCtl *ctl)
{
	if (!ctl)
		return;

	process_events(ctl);
}

TVoutCtl *tvout_ctl_init(TVoutCtlNotify ui_notify, void *ui_data)
{
	TVoutCtl *ctl;

	ctl = calloc(1, sizeof *ctl);
	if (!ctl)
		return NULL;

	if (!rr_init(ctl)) {
		free(ctl);
		return NULL;
	}

	if (!probe_outputs(ctl)) {
		rr_exit(ctl);
		free(ctl);
		return NULL;
	}

	if (!init_properties(ctl)) {
		rr_exit(ctl);
		free(ctl);
		return NULL;
	}

	process_events(ctl);

	ctl->ui_notify = ui_notify;
	ctl->ui_data = ui_data;

	return ctl;
}

void tvout_ctl_exit(TVoutCtl *ctl)
{
	if (!ctl)
		return;

	free_properties(ctl);
	rr_exit(ctl);
	free(ctl);
}
