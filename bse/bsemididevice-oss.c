/* BSE - Bedevilled Sound Engine
 * Copyright (C) 1996-2003 Tim Janik
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#include	"bsemididevice-oss.h"

#include	"bseserver.h"
#include	"bsemididecoder.h"
#include	"gslcommon.h"

#include	"topconfig.h"

#ifndef	BSE_MIDI_DEVICE_CONF_OSS
BSE_DUMMY_TYPE (BseMidiDeviceOSS);
#else   /* BSE_MIDI_DEVICE_CONF_OSS */

#if HAVE_SYS_SOUNDCARD_H
#include	<sys/soundcard.h>
#elif HAVE_SOUNDCARD_H
#include	<soundcard.h>
#endif
#include	<sys/ioctl.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<fcntl.h>


/* --- structs --- */
typedef struct
{
  BseMidiHandle	handle;
  gint		fd;
} OSSHandle;


/* --- prototypes --- */
static void	    io_handler				(BseMidiDevice		*mdev,
							 GPollFD		*pfd);

/* --- variables --- */
static gpointer parent_class = NULL;


/* --- functions --- */
static void
bse_midi_device_oss_init (BseMidiDeviceOSS *oss)
{
  oss->device_name = g_strdup (BSE_MIDI_DEVICE_CONF_OSS);
}

static SfiRing*
bse_midi_device_oss_list_devices (BseDevice *device)
{
  const gchar *postfixes[] = { "", "0", "1", "2", "3" };
  SfiRing *ring = NULL;
  guint i;
  gchar *last = NULL;
  for (i = 0; i < G_N_ELEMENTS (postfixes); i++)
    {
      gchar *dname = g_strconcat (BSE_MIDI_DEVICE_OSS (device)->device_name, postfixes[i], NULL);
      if (!gsl_check_file_equals (last, dname))
        {
          if (gsl_check_file (dname, "crw") == GSL_ERROR_NONE)
            ring = sfi_ring_append (ring,
                                    bse_device_entry_new (device,
                                                          g_strdup_printf ("%s,rw", dname),
                                                          g_strdup_printf ("%s (read-write)", dname)));
          else if (gsl_check_file (dname, "cr") == GSL_ERROR_NONE)
            ring = sfi_ring_append (ring,
                                    bse_device_entry_new (device,
                                                          g_strdup_printf ("%s,ro", dname),
                                                          g_strdup_printf ("%s (read only)", dname)));
          else if (gsl_check_file (dname, "cw") == GSL_ERROR_NONE)
            ring = sfi_ring_append (ring,
                                    bse_device_entry_new (device,
                                                          g_strdup_printf ("%s,wo", dname),
                                                          g_strdup_printf ("%s (write only)", dname)));
        }
      g_free (last);
      last = dname;
    }
  g_free (last);
  if (!ring)
    ring = sfi_ring_append (ring, bse_device_error_new (device, g_strdup_printf ("No devices found")));
  return ring;
}

static BseErrorType
bse_midi_device_oss_open (BseDevice     *device,
                          gboolean       require_readable,
                          gboolean       require_writable,
                          guint          n_args,
                          const gchar  **args)
{
  const gchar *dname;
  if (n_args >= 1)      /* DEVICE */
    dname = args[0];
  else
    dname = BSE_MIDI_DEVICE_OSS (device)->device_name;
  gint omode, retry_mode = 0;
  if (n_args >= 2)      /* MODE */
    omode = strcmp (args[1], "rw") == 0 ? O_RDWR : strcmp (args[1], "ro") == 0 ? O_RDONLY : O_WRONLY;   /* parse: ro rw wo */
  else
    {
      omode = O_RDWR;
      retry_mode = O_RDONLY;
    }
  OSSHandle *oss = g_new0 (OSSHandle, 1);
  BseMidiHandle *handle = &oss->handle;
  
  /* setup request */
  oss->fd = -1;

  /* try open */
  BseErrorType error;
  gint fd = -1;
  handle->readable = (omode & O_RDWR) == O_RDWR || (omode & O_RDONLY) == O_RDONLY;
  handle->writable = (omode & O_RDWR) == O_RDWR || (omode & O_WRONLY) == O_WRONLY;
  if ((handle->readable || !require_readable) && (handle->writable || !require_writable))
    fd = open (dname, omode | O_NONBLOCK, 0);           /* open non blocking to avoid waiting for other clients */
  if (fd < 0 && retry_mode)
    {
      omode = retry_mode;
      handle->writable = (omode & O_RDWR) == O_RDWR || (omode & O_WRONLY) == O_WRONLY;
      handle->readable = (omode & O_RDWR) == O_RDWR || (omode & O_RDONLY) == O_RDONLY;
      if ((handle->readable || !require_readable) && (handle->writable || !require_writable))
        fd = open (dname, omode | O_NONBLOCK, 0);       /* open non blocking to avoid waiting for other clients */
    }
  if (fd >= 0)
    {
      oss->fd = fd;
      /* try setup */
      error = BSE_ERROR_NONE;
    }
  else
    error = bse_error_from_errno (errno, BSE_ERROR_FILE_OPEN_FAILED);
  
  /* setup mdev or shutdown */
  if (!error)
    {
      BSE_OBJECT_SET_FLAGS (device, BSE_DEVICE_FLAG_OPEN);
      if (handle->readable)
	BSE_OBJECT_SET_FLAGS (device, BSE_DEVICE_FLAG_READABLE);
      if (handle->writable)
	BSE_OBJECT_SET_FLAGS (device, BSE_DEVICE_FLAG_WRITABLE);
      BSE_MIDI_DEVICE (device)->handle = handle;
      handle->midi_fd = oss->fd;
      bse_server_add_io_watch (bse_server_get (), handle->midi_fd, G_IO_IN, (BseIOWatch) io_handler, device);
    }
  else
    {
      if (oss->fd < 0)
	close (oss->fd);
      g_free (oss);
    }
  
  return error;
}

static void
bse_midi_device_oss_close (BseDevice *device)
{
  OSSHandle *oss = (OSSHandle*) BSE_MIDI_DEVICE (device)->handle;
  BseMidiHandle *handle = &oss->handle;
  BSE_MIDI_DEVICE (device)->handle = NULL;
  
  g_assert (handle->running_thread == FALSE);
  /* midi_handle_abort_wait (handle); */
  
  bse_server_remove_io_watch (bse_server_get (), (BseIOWatch) io_handler, device);
  (void) close (oss->fd);
  g_free (oss);
}

static void
bse_midi_device_oss_finalize (GObject *object)
{
  BseMidiDeviceOSS *mdev_oss = BSE_MIDI_DEVICE_OSS (object);
  
  g_free (mdev_oss->device_name);
  mdev_oss->device_name = NULL;
  
  /* chain parent class' handler */
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
io_handler (BseMidiDevice *mdev,
	    GPollFD       *pfd)
{
  OSSHandle *oss = (OSSHandle*) mdev->handle;
  BseMidiHandle *handle = &oss->handle;
  const gsize buf_size = 8192;
  guint8 buffer[buf_size];
  guint64 systime;
  gssize l;
  
  /* this should spawn its own thread someday */
  g_assert (handle->running_thread == FALSE);
  
  systime = sfi_time_system ();
  do
    l = read (oss->fd, buffer, buf_size);
  while (l < 0 && errno == EINTR);	/* don't mind signals */
  
  if (l > 0)
    bse_midi_decoder_push_data (mdev->midi_decoder, l, buffer, systime);
}

static void
bse_midi_device_oss_class_init (BseMidiDeviceOSSClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  BseDeviceClass *device_class = BSE_DEVICE_CLASS (class);

  parent_class = g_type_class_peek_parent (class);

  gobject_class->finalize = bse_midi_device_oss_finalize;

  device_class->list_devices = bse_midi_device_oss_list_devices;
  bse_device_class_setup (class,
                          BSE_RATING_DEFAULT,
                          "oss", "DEVICE,MODE",
                          /* TRANSLATORS: keep this text to 70 chars in width */
                          _("Open Sound System MIDI driver:\n"
                            "DEVICE - MIDI device file name.\n"
                            "MODE   - one of \"ro\", \"rw\" or \"wo\" for\n"
                            "         read-only, read-write or write-only access."));
  device_class->open = bse_midi_device_oss_open;
  device_class->close = bse_midi_device_oss_close;
}

BSE_BUILTIN_TYPE (BseMidiDeviceOSS)
{
  GType midi_device_oss_type;
  
  static const GTypeInfo midi_device_oss_info = {
    sizeof (BseMidiDeviceOSSClass),
    
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) bse_midi_device_oss_class_init,
    (GClassFinalizeFunc) NULL,
    NULL /* class_data */,
    
    sizeof (BseMidiDeviceOSS),
    0 /* n_preallocs */,
    (GInstanceInitFunc) bse_midi_device_oss_init,
  };
  
  midi_device_oss_type = bse_type_register_static (BSE_TYPE_MIDI_DEVICE,
						   "BseMidiDeviceOSS",
						   "MIDI device implementation for OSS Lite /dev/dsp",
						   &midi_device_oss_info);
  return midi_device_oss_type;
}

#endif	/* BSE_MIDI_DEVICE_CONF_OSS */
