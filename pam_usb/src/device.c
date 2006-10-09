/*
 * Copyright (c) 2003-2006 Andrea Luzzardi <scox@sig11.org>
 *
 * This file is part of the pam_usb project. pam_usb is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * pam_usb is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <unistd.h>
#include <string.h>
#include <dbus/dbus.h>
#include <libhal-storage.h>
#include "conf.h"
#include "hal.h"
#include "log.h"
#include "otp.h"
#include "device.h"

static int	pusb_device_connected(t_pusb_options *opts, LibHalContext *ctx)
{
  char		*udi = NULL;

  log_debug("Searching for \"%s\" in the hardware database...\n",
	    opts->device.name);
  udi = pusb_hal_find_item(ctx,
			   "usb_device.serial", opts->device.serial,
			   "usb_device.vendor", opts->device.vendor,
			   "info.product", opts->device.model,
			   NULL);
  if (!udi)
    {
      log_error("Device \"%s\" is not connected.\n",
		opts->device.name);
      return (0);
    }
  libhal_free_string(udi);
  log_info("Device \"%s\" is connected (good).\n", opts->device.name);
  return (1);
}

int			pusb_device_check(t_pusb_options *opts)
{
  DBusConnection	*dbus = NULL;
  LibHalContext		*ctx = NULL;
  int			retval = 0;

  log_debug("Connecting to HAL...\n");
  if (!(dbus = pusb_hal_dbus_connect()))
    return (0);

  if (!(ctx = pusb_hal_init(dbus)))
    {
      pusb_hal_dbus_disconnect(dbus);
      return (0);
    }

  if (!pusb_device_connected(opts, ctx))
    {
      pusb_hal_dbus_disconnect(dbus);
      libhal_ctx_free(ctx);
      return (0);
    }

  if (opts->one_time_pad)
    {
      log_info("Performing one time pad verification...\n");
      retval = pusb_pad_check(opts, ctx);
    }
  else
    {
      log_debug("One time pad is disabled, no more verifications to do.\n");
      retval = 1;
    }

  pusb_hal_dbus_disconnect(dbus);
  libhal_ctx_free(ctx);
  return (retval);
}
